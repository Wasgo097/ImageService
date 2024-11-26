#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/strings/str_format.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "cmake/build/imageService.grpc.pb.h"
#include <opencv4/opencv2/opencv.hpp>
using namespace std::chrono_literals;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
enum class CallStatus
{
    CREATE,
    PROCESS,
    FINISH
};

class CallDataBase
{
public:
    virtual ~CallDataBase() = default;
    virtual void Proceed() = 0;
};

template <class RequestType, class ReplyType>
class CallDataT : CallDataBase
{
public:
    CallDataT(ImageService::NewImageService::AsyncService *service,
              grpc::ServerCompletionQueue *completionQueue)
        : status_(CallStatus::CREATE),
          service_(service),
          completionQueue_(completionQueue),
          responder_(&serverContext_) {}
    virtual ~CallDataT() = default;

    void Proceed() override
    {
        if (status_ == CallStatus::CREATE)
        {
            status_ = CallStatus::PROCESS;
            WaitForRequest();
        }
        else if (status_ == CallStatus::PROCESS)
        {
            AddNextToCompletionQueue();
            HandleRequest();
            status_ = CallStatus::FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        }
        else
        {
            // We're done! Self-destruct!
            if (status_ != CallStatus::FINISH)
            {
                // Log some error message
            }
            delete this;
        }
    }

protected:
    virtual void AddNextToCompletionQueue() = 0;
    virtual void WaitForRequest() = 0;
    virtual void HandleRequest() = 0;

    CallStatus status_;
    ImageService::NewImageService::AsyncService *service_;
    grpc::ServerCompletionQueue *completionQueue_;
    RequestType request_;
    ReplyType reply_;
    grpc::ServerAsyncResponseWriter<ReplyType> responder_;
    grpc::ServerContext serverContext_;
};

class TestConnectionProcessor : CallDataT<ImageService::TestRequest, ImageService::TestReply>
{
public:
    TestConnectionProcessor(ImageService::NewImageService::AsyncService *service,
                            grpc::ServerCompletionQueue *completionQueue)
        : CallDataT(service, completionQueue)
    {
        Proceed();
    }

protected:
    void AddNextToCompletionQueue() override
    {
        new TestConnectionProcessor(service_, completionQueue_);
    }

    void WaitForRequest() override
    {
        service_->RequestTestConnection(&serverContext_, &request_, &responder_,
                                        completionQueue_, completionQueue_, this);
    }

    void HandleRequest() override
    {
        auto mess = "Yo " + request_.message();
        std::cout << mess << std::endl;
        reply_.set_message(mess);
    }
};

class NewImageProcessor : CallDataT<ImageService::ImageRequest, ImageService::ImageReply>
{
public:
    NewImageProcessor(ImageService::NewImageService::AsyncService *service,
                      grpc::ServerCompletionQueue *completionQueue)
        : CallDataT(service, completionQueue)
    {
        Proceed();
    }

protected:
    void AddNextToCompletionQueue() override
    {
        new NewImageProcessor(service_, completionQueue_);
    }

    void WaitForRequest() override
    {
        service_->RequestNewImage(&serverContext_, &request_, &responder_,
                                  completionQueue_, completionQueue_, this);
    }

    void HandleRequest() override
    {
        int height = request_.height();
        int width = request_.width();
        int type = request_.type();
        auto data = request_.data();
        std::cout << "Received " << data.size() << " bytes" << std::endl;
        reply_.set_message("OK");
        if ((height > 0) && (width > 0))
        {
            _image = cv::Mat(height, width, type, const_cast<char *>(data.c_str())).clone();
            cv::imshow("Service", _image);
            cv::waitKey(500);
        }
    }

    cv::Mat _image;
};

class ServerImpl final
{
public:
    ~ServerImpl()
    {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run(uint16_t port)
    {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;
        HandleRpcs();
    }

private:
    void HandleRpcs()
    {
        new TestConnectionProcessor(&service_, cq_.get());
        new NewImageProcessor(&service_, cq_.get());
        void *tag = nullptr;
        bool ok;
        while (true)
        {
            CHECK(cq_->Next(&tag, &ok));
            CHECK(ok);
            std::cout << "Next tag\n";
            static_cast<CallDataBase *>(tag)->Proceed();
        }
    }

    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    ImageService::NewImageService::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;
};

int main(int argc, char **argv)
{
    absl::ParseCommandLine(argc, argv);
    ServerImpl server;
    server.Run(absl::GetFlag(FLAGS_port));
    return 0;
}
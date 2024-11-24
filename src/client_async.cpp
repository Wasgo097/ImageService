#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "cmake/build/imageService.grpc.pb.h"
#include <opencv4/opencv2/opencv.hpp>

using namespace std::chrono_literals;
ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

class Client
{
public:
    explicit Client(std::shared_ptr<grpc::Channel> channel) : stub_(ImageService::NewImageService::NewStub(channel))
    {
        buffer_ = cv::Mat(HEIGHT, WIDTH, TYPE);
    }

    std::string TestConnection(const std::string &user)
    {
        grpc::ClientContext context;
        ImageService::TestRequest request;
        ImageService::TestReply reply;
        grpc::Status status;
        request.set_message(user);

        std::unique_ptr<grpc::ClientAsyncResponseReader<ImageService::TestReply>> rpc(
            stub_->AsyncTestConnection(&context, request, &cq));
        rpc->Finish(&reply, &status, (void *)1);
        void *tag = nullptr;
        bool ok = false;
        CHECK(cq.Next(&tag, &ok));
        CHECK_EQ(tag, (void *)1);
        CHECK(ok);
        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            return "RPC failed";
        }
        return "";
    }

    std::string NewImage()
    {
        grpc::ClientContext context;
        ImageService::ImageRequest request;
        ImageService::ImageReply reply;
        grpc::Status status;
        generateNoiseImage();
        request.set_height(HEIGHT);
        request.set_width(WIDTH);
        request.set_type(TYPE);
        auto buffer = parseImage();
        std::cout << "To send " << buffer.size() << " bytes" << std::endl;
        request.set_data(buffer);
        std::unique_ptr<grpc::ClientAsyncResponseReader<ImageService::ImageReply>> rpc(
            stub_->AsyncNewImage(&context, request, &cq));
        rpc->Finish(&reply, &status, (void *)1);
        void *tag = nullptr;
        bool ok = false;
        CHECK(cq.Next(&tag, &ok));
        CHECK_EQ(tag, (void *)1);
        CHECK(ok);
        if (status.ok())
            return reply.message();
        else
        {
            std::cout << "Error " << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
        return "";
    }

private:
    void generateNoiseImage()
    {
        cv::randu(buffer_, cv::Scalar::all(0), cv::Scalar::all(255));
    }

    std::string parseImage() const
    {
        std::string buffer;
        if (!buffer_.isContinuous())
        {
            static cv::Mat continuousMat = buffer_.clone();
            buffer.insert(buffer.end(), continuousMat.data, continuousMat.data + continuousMat.total() * continuousMat.elemSize());
        }
        else
            buffer.insert(buffer.end(), buffer_.data, buffer_.data + buffer_.total() * buffer_.elemSize());
        return buffer;
    }

    std::unique_ptr<ImageService::NewImageService::Stub> stub_;

    grpc::CompletionQueue cq;
    cv::Mat buffer_;
    static constexpr int WIDTH = 640;
    static constexpr int HEIGHT = 480;
    static constexpr int TYPE = CV_8UC3;
};

int main(int argc, char **argv)
{
    absl::ParseCommandLine(argc, argv);
    std::string targetStr = absl::GetFlag(FLAGS_target);
    Client client(grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials()));
    std::cout << "Client received: " << client.TestConnection("User") << std::endl;
    for (int i = 0; i < 5; i++)
    {
        std::string reply = client.NewImage();
        std::cout << "Client received: " << reply << std::endl;
        if (reply != "OK")
            break;
    }
    return 0;
}
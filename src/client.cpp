#include <chrono>
#include <iostream>
#include <thread>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/grpcpp.h>

#include <opencv4/opencv2/opencv.hpp>

#include "cmake/build/imageService.grpc.pb.h"

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

class Client
{
public:
    Client(std::shared_ptr<grpc::Channel> channel) : stub_(ImageService::NewImageService::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string NewImage()
    {
        grpc::ClientContext context;
        ImageService::ImageRequest request;
        ImageService::ImageReply reply;
        buffer_ = generateNoiseImage(WIDTH, HEIGHT, TYPE);
        request.set_height(HEIGHT);
        request.set_width(WIDTH);
        request.set_type(TYPE);
        auto buffer = parseImage(buffer_);
        std::cout << "To send " << buffer.size() << " bytes" << std::endl;
        request.set_data(buffer);
        auto data = request.data();
        std::cout << "Real data size " << data.size() << std::endl;
        grpc::Status status = stub_->NewImage(&context, request, &reply);
        if (status.ok())
            return reply.message();
        else
        {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

    std::string TestConnection()
    {
        grpc::ClientContext context;
        ImageService::Test request;
        ImageService::Test reply;
        request.set_message("Test message");
        grpc::Status status = stub_->TestConnection(&context, request, &reply);

        if (status.ok())
            return reply.message();
        else
        {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

private:
    cv::Mat generateNoiseImage(int width, int height, int type) const
    {
        cv::Mat noiseImage(height, width, type);
        cv::randu(noiseImage, cv::Scalar::all(0), cv::Scalar::all(255));
        return noiseImage;
    }

    std::string parseImage(const cv::Mat &mat) const
    {
        std::string buffer;
        if (!mat.isContinuous())
        {
            static cv::Mat continuousMat = mat.clone();
            buffer.insert(buffer.end(), continuousMat.data, continuousMat.data + continuousMat.total() * continuousMat.elemSize());
        }
        else
            buffer.insert(buffer.end(), mat.data, mat.data + mat.total() * mat.elemSize());
        return buffer;
    }

    std::unique_ptr<ImageService::NewImageService::Stub> stub_;
    cv::Mat buffer_;
    static constexpr int WIDTH = 640;
    static constexpr int HEIGHT = 480;
    static constexpr int TYPE = CV_8UC3;
};

using namespace std::chrono_literals;
int main(int argc, char **argv)
{
    //static constexpr int MAX_DATA_LENGTH = 200; // in MB
    absl::ParseCommandLine(argc, argv);
    std::string target_str = absl::GetFlag(FLAGS_target);
    grpc::ChannelArguments ch_args;
    // ch_args.SetMaxReceiveMessageSize(MAX_DATA_LENGTH * 1024 * 1024);
    // ch_args.SetMaxSendMessageSize(MAX_DATA_LENGTH * 1024 * 1024);
    Client client(grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), ch_args));
    std::cout << "Test connection: " << client.TestConnection() << std::endl;
    for (int i = 0; i < 5; i++)
    {
        std::string reply = client.NewImage();
        std::cout << "Client received: " << reply << std::endl;
        if (reply != "OK")
            break;
        //std::this_thread::sleep_for(2000ms);
    }
    return 0;
}

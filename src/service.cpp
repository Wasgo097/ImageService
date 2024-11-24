// #include <iostream>
// #include <memory>
// #include <string>

// #include "absl/flags/flag.h"
// #include "absl/flags/parse.h"
// #include "absl/strings/str_format.h"

// #include <grpcpp/grpcpp.h>
// #include <grpcpp/ext/proto_server_reflection_plugin.h>
// #include <grpcpp/health_check_service_interface.h>

// #include <opencv4/opencv2/opencv.hpp>

// #include "cmake/build/imageService.grpc.pb.h"

// //#include "StopWatch.h"

// ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

// class Service : public ImageService::NewImageService::Service
// {

// protected:
//     virtual grpc::Status NewImage(grpc::ServerContext *context, const ImageService::ImageRequest *request, ImageService::ImageReply *response) override
//     {
//         std::ignore = context;
//         std::cout << "New image" << std::endl;
//         // Get the Mat back
//         int height = request->height();
//         int width = request->width();
//         int type = request->type();
//         std::cout << "Width : " << width << std::endl;
//         std::cout << "Height: " << height << std::endl;
//         std::cout << "Type: " << type << std::endl;
//         if ((height > 0) && (width > 0))
//         {
//             auto data = request->data();
//             std::cout << "Received " << data.size() << " bytes" << std::endl;
//             _image = cv::Mat(height, width, type, const_cast<char *>(data.c_str())).clone();
//             cv::imshow("Service", _image);
//             cv::waitKey(1000);
//         }
//         response->set_message("OK");
//         return ::grpc::Status::OK;
//     }

//     virtual grpc::Status TestConnection(grpc::ServerContext *context, const ImageService::Test *request, ImageService::Test *response)
//     {
//         std::ignore = context;
//         std::cout << "Test message: " << request->message() << std::endl;
//         response->set_message("return ok");
//         return grpc::Status::OK;
//     }

//     cv::Mat _image;
// };

// void RunServer(uint16_t port)
// {
//     // static constexpr int MAX_DATA_LENGTH = 200; // in MB
//     std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
//     Service service;

//     grpc::EnableDefaultHealthCheckService(true);
//     grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//     grpc::ServerBuilder builder;
//     builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//     // builder.SetMaxReceiveMessageSize(MAX_DATA_LENGTH * 1024 * 1024);
//     // builder.SetMaxSendMessageSize(MAX_DATA_LENGTH * 1024 * 1024);
//     builder.RegisterService(&service);
//     std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
//     std::cout << "Server listening on " << server_address << std::endl;
//     server->Wait();
// }

int main(int argc, char **argv)
{
    // absl::ParseCommandLine(argc, argv);
    // RunServer(absl::GetFlag(FLAGS_port));
    return 0;
}
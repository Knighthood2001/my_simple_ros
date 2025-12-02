//
// Created by wu on 25-12-2.
//
#include <iostream>
#include "ros_rpc_client.h"

namespace simple_ros{
RosRpcClient::RosRpcClient(const std::string& server_address){
  stub_ = RosRpcService::NewStub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
}
}
/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <gtest/gtest.h>
#include <cpp_redis/cpp_redis>

template <typename value_type>
void SerializeToStream(std::ostream& os, const value_type* value, size_t size) {
  os.write(reinterpret_cast<const char*>(&size), sizeof(size));
  for (size_t i = 0; i < size; ++i) {
    os.write(reinterpret_cast<const char*>(&value[i]), sizeof(value_type));
  }
}

template <typename value_type>
void DeserializeFromStream(std::istream& is, value_type* buf, size_t size) {
  size_t actual_size;
  is.read(reinterpret_cast<char*>(&actual_size), sizeof(actual_size));
  assert(actual_size == size);
  for (size_t i = 0; i < size; ++i) {
    is.read(reinterpret_cast<char*>(&buf[i]), sizeof(value_type));
  }
}

TEST(CppRedis, Serialize) {
  size_t length = 3;
  double a[3] = {1.23456, 2.7891234, 3.16493272};

  std::ostringstream oss;
  SerializeToStream(oss, a, length);

  double b[3];

  std::istringstream iss(oss.str());
  DeserializeFromStream(iss, b, length);
  for (size_t i = 0; i < length; ++i) {
    EXPECT_EQ(b[i], a[i]);
  }
}

TEST(CppRedis, PutGet) {
  cpp_redis::client client;

  client.connect("127.0.0.1", 6379,
                 [](const std::string& host, std::size_t port,
                    cpp_redis::client::connect_state status) {
                   if (status == cpp_redis::client::connect_state::dropped) {
                     std::cout << "client disconnected from " << host << ":"
                               << port << std::endl;
                   }
                 });

  client.flushall([](cpp_redis::reply& reply) {});

  client.exists({"a"},
                [](cpp_redis::reply& reply) {
                  int64_t exist = reply.as_integer();
                  EXPECT_EQ(exist, 0);
                });

  double a[3] = {1.23456, 2.7891234, 3.16493272};
  std::ostringstream oss;
  SerializeToStream(oss, a, 3);
  client.set("a", oss.str(), [](cpp_redis::reply& reply) {});

  client.exists({"a"},
                [](cpp_redis::reply& regply) {
                  int64_t exist = reply.as_integer();
                  EXPECT_EQ(exist, 1);
                });

  double b[3];
  client.get("a", [&a, &b](cpp_redis::reply& reply) {
    std::istringstream iss(reply.as_string());
    DeserializeFromStream(iss, b, 3);
    for (size_t i = 0; i < 3; ++i) {
      std::cout << b[i] << std::endl;
      EXPECT_EQ(b[i], a[i]);
    }
  });

  client.sync_commit();
  client.shutdown();
}

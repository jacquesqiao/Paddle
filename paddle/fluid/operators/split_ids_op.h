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

#pragma once

#include <vector>
#include "paddle/fluid/framework/op_registry.h"
#include "paddle/fluid/operators/math/selected_rows_functor.h"

namespace paddle {
namespace operators {

static int FindOutIdx(int row, const std::vector<int>& abs_sections) {
  for (size_t i = 1; i < abs_sections.size(); ++i) {
    if (row < abs_sections[i]) {
      return i - 1;
    }
  }
  return abs_sections.size() - 1;
}

static std::vector<int> ToAbsoluteSection(
    const std::vector<int>& height_sections) {
  std::vector<int> abs_sections;
  abs_sections.resize(height_sections.size());
  abs_sections[0] = 0;
  for (size_t i = 1; i < height_sections.size(); ++i) {
    abs_sections[i] = height_sections[i - 1] + abs_sections[i - 1];
  }
  return abs_sections;
}

template <typename DeviceContext, typename T>
class SplitIdsOpKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& ctx) const override {
    auto place = ctx.GetPlace();
    if (!platform::is_cpu_place(place)) {
      PADDLE_THROW("SplitIds do not support GPU kernel");
    }

    const auto* ids_t = ctx.Input<framework::LoDTensor>("Ids");
    auto& ids_dims = ids_t->dims();
    auto outs = ctx.MultiOutput<framework::LoDTensor>("Out");

    const int64_t* ids = ids_t->data<int64_t>();

    const size_t shard_num = outs.size();

    std::vector<std::vector<int64_t>> out_ids;
    out_ids.resize(outs.size());

    // split id by their shard_num.
    for (size_t i = 0; i < ids_dims[0]; ++i) {
      int64_t id = ids[i];
      size_t shard_id = id % shard_num;
      out_ids[shard_id].push_back(id);
    }

    // create tensor for each shard and send to parameter server
    for (size_t i = 0; i < out_ids.size(); ++i) {
      auto* shard_t = outs[i];
      std::vector<int64_t> ids = out_ids[i];
      auto* shard_data = shard_t->mutable_data<int64_t>(
          framework::make_ddim({static_cast<int64_t>(ids.size()), 1}), place);
      for (size_t i = 0; i < ids.size(); ++i) {
        shard_data[i] = ids[i];
      }
    }
  }
};

}  // namespace operators
}  // namespace paddle

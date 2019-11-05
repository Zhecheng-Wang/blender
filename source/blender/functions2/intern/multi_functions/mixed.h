#pragma once

#include "FN_multi_function.h"

namespace FN {

class MF_AddFloats final : public MultiFunction {
 public:
  MF_AddFloats();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_AddFloat3s final : public MultiFunction {
 public:
  MF_AddFloat3s();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_CombineVector final : public MultiFunction {
 public:
  MF_CombineVector();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_SeparateVector final : public MultiFunction {
 public:
  MF_SeparateVector();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_VectorDistance final : public MultiFunction {
 public:
  MF_VectorDistance();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_FloatArraySum final : public MultiFunction {
 public:
  MF_FloatArraySum();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_FloatRange final : public MultiFunction {
 public:
  MF_FloatRange();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_ObjectWorldLocation final : public MultiFunction {
 public:
  MF_ObjectWorldLocation();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_TextLength final : public MultiFunction {
 public:
  MF_TextLength();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

template<typename T> class MF_ConstantValue : public MultiFunction {
 private:
  T m_value;

 public:
  MF_ConstantValue(T value) : m_value(std::move(value))
  {
    MFSignatureBuilder signature("Constant " + GET_TYPE<T>().name());
    signature.single_output<T>("Output");
    this->set_signature(signature);
  }

  void call(const MFMask &mask, MFParams &params, MFContext &UNUSED(context)) const override
  {
    MutableArrayRef<T> output = params.single_output<T>(0, "Output");

    mask.foreach_index([&](uint i) { new (output.begin() + i) T(m_value); });
  }
};

template<typename FromT, typename ToT> class MF_Convert : public MultiFunction {
 public:
  MF_Convert()
  {
    MFSignatureBuilder signature(GET_TYPE<FromT>().name() + " to " + GET_TYPE<ToT>().name());
    signature.readonly_single_input<FromT>("Input");
    signature.single_output<ToT>("Output");
    this->set_signature(signature);
  }

  void call(const MFMask &mask, MFParams &params, MFContext &UNUSED(context)) const override
  {
    VirtualListRef<FromT> inputs = params.readonly_single_input<FromT>(0, "Input");
    MutableArrayRef<ToT> outputs = params.single_output<ToT>(1, "Output");

    for (uint i : mask.indices()) {
      const FromT &from_value = inputs[i];
      new (outputs.begin() + i) ToT(from_value);
    }
  }
};

class MF_SimpleVectorize final : public MultiFunction {
 private:
  const MultiFunction &m_function;
  Vector<bool> m_input_is_vectorized;
  Vector<uint> m_vectorized_inputs;
  Vector<uint> m_output_indices;

 public:
  MF_SimpleVectorize(const MultiFunction &function, ArrayRef<bool> input_is_vectorized);
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_ContextVertexPosition final : public MultiFunction {
 public:
  MF_ContextVertexPosition();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

class MF_ContextCurrentFrame final : public MultiFunction {
 public:
  MF_ContextCurrentFrame();
  void call(const MFMask &mask, MFParams &params, MFContext &context) const override;
};

template<typename FromT, typename ToT, ToT (*Compute)(const FromT &)>
class MF_Mappping final : public MultiFunction {
 public:
  MF_Mappping(StringRef name)
  {
    MFSignatureBuilder signature(name);
    signature.readonly_single_input<FromT>("Input");
    signature.single_output<ToT>("Output");
    this->set_signature(signature);
  }

  void call(const MFMask &mask, MFParams &params, MFContext &UNUSED(context)) const override
  {
    VirtualListRef<FromT> inputs = params.readonly_single_input<FromT>(0, "Input");
    MutableArrayRef<ToT> outputs = params.single_output<ToT>(1, "Output");

    for (uint i : mask.indices()) {
      const FromT &from_value = inputs[i];
      ToT to_value = Compute(from_value);
      new (&outputs[i]) ToT(std::move(to_value));
    }
  }
};

template<typename T, T (*Compute)(T, T)> class MF_SimpleMath final : public MultiFunction {
 private:
  uint m_input_amount;

 public:
  MF_SimpleMath(StringRef name, uint input_amount) : m_input_amount(input_amount)
  {
    BLI_assert(input_amount >= 1);
    MFSignatureBuilder signature(name);
    for (uint i = 0; i < m_input_amount; i++) {
      signature.readonly_single_input<T>("Input");
    }
    signature.single_output<T>("Output");
    this->set_signature(signature);
  }

  void call(const MFMask &mask, MFParams &params, MFContext &UNUSED(context)) const override
  {
    MutableArrayRef<T> outputs = params.single_output<T>(m_input_amount, "Output");

    if (m_input_amount == 1) {
      VirtualListRef<T> inputs = params.readonly_single_input<T>(0, "Input");
      for (uint i : mask.indices()) {
        outputs[i] = inputs[i];
      }
    }
    else {
      BLI_assert(m_input_amount >= 2);
      VirtualListRef<T> inputs0 = params.readonly_single_input<T>(0, "Input");
      VirtualListRef<T> inputs1 = params.readonly_single_input<T>(1, "Input");

      for (uint i : mask.indices()) {
        outputs[i] = Compute(inputs0[i], inputs1[i]);
      }

      for (uint param_index = 2; param_index < m_input_amount; param_index++) {
        VirtualListRef<T> inputs = params.readonly_single_input<T>(param_index, "Input");
        for (uint i : mask.indices()) {
          outputs[i] = Compute(outputs[i], inputs[i]);
        }
      }
    }
  }
};

}  // namespace FN
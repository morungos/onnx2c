#pragma once
#include <string>
#include <vector>
#include "onnx.pb.h"
#include "tensor.h"

/* ONNX names are not valid C - make name acceptable to the C compiler*/
std::string cify_name(const std::string &in);

/* Helper functions to parse attributes in a onnx NodeProto */
int parse_attribute_int(const onnx::AttributeProto &a);
std::vector<int64_t> parse_attribute_ints(const onnx::AttributeProto &a);
float parse_attribute_float(const onnx::AttributeProto &a);
std::vector<float> parse_attribute_floats(const onnx::AttributeProto &a);
std::string parse_attribute_string(const onnx::AttributeProto &a);
std::vector<std::string> parse_attribute_strings(const onnx::AttributeProto &a);
toC::Tensor* parse_attribute_tensor(const onnx::AttributeProto &a);

/* Wrap all constant accesses into with this function.
 * If targetting AVR, the constants are stored in another memory space than data,
 * this wrapper takes care of that.
 * NB: this is a late add-on. Not all nodes are using this
 */
std::string constant_acces_code(const std::string plain);

/*
 * Cast a function parameter name to a more readable "shortname".
 * I.e. returns a string like:
 * float (*X)[1][2] = (float (*)[1][2])tensor_123;
 */
std::string cast_to_ndim_arrayptr(const toC::Tensor *t, const std::string shortname);

#define INDT_1 dst<<"\t"
#define INDT_2 dst<<"\t\t"
#define INDT_3 dst<<"\t\t\t"
#define INDT_4 dst<<"\t\t\t\t"
#define INDT_5 dst<<"\t\t\t\t\t"
#define INDT(X) {for(unsigned _i=0;_i<(X); _i++) INDT_1;} dst

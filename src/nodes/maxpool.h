/* This file is part of onnx2c.
 *
 * MaxPool
 * Picks maximum value from the elements that
 * are under the kernel.
 *
 */
#include "pooling.h"
namespace toC {

class MaxPool : public Pooling {
	public:
	MaxPool() {
		op_name = "MaxPool";
		Indices=NULL;
	}

	// optional outputs
	const Tensor *Indices;

	std::vector<int> pad_shapes;

	virtual void print_parameters(std::ostream &dst, bool decorate ) const override
	{
		x->print_tensor_as_const(dst, !decorate);
		dst << ", ";
		y->print_tensor(dst, !decorate);
		if( Indices->name != "" ) {
			dst << ", ";
			Indices->print_tensor(dst, !decorate);
		}
	}

	virtual void parseAttributes( onnx::NodeProto &node ) override {

		Pooling::parseAttributes(node);

		for( const auto& a : node.attribute() ) {
			if( a.name() == "storage_order" )
				ERROR("Unimplemented: MaxPool storage_order attribute");
		}
	}

	virtual void print_output_cell_init(std::ostream &dst, const std::string &y_idx) const override
	{
		std::string type = x->data_type_str();
		std::string type_min_value;
		if( type == "float" )
			type_min_value = "-FLT_MAX";
		else if( type == "int8_t" )
			type_min_value = "INT8_MIN";
		else if( type == "uint8_t" )
			type_min_value = "0";
		else if( type == "int32_t" )
			type_min_value = "INT32_MIN";
		else
			ERROR("Unimplemented: minimum value for this type");

		INDT_3  << type << " curmax = " << type_min_value << ";" << std::endl;
		if( Indices->name != "" )
			INDT_3 << "int64_t curmaxind = -1;" << std::endl;


	}
	virtual void print_output_cell_calc(
		std::ostream &dst,
		const std::string &x_idx,
		const std::string &w_idx,
		const std::string &y_idx) const override
	{
		unsigned n_data_dims = x->data_dim.size()-2;
		// Calculate how much one index means in terms of the Indices output.
		// Generate helper string for the next step.
		std::vector<int>size_of_dim(x->rank());
		size_of_dim[x->data_dim.size()-1]=1;
		for( int i=x->data_dim.size()-2; i>= 0; i--)
			size_of_dim[i] = size_of_dim[i+1] * x->data_dim[i];
		std::string indices_value = "(b*" + std::to_string(size_of_dim[0]) + ")+(c*" + std::to_string(size_of_dim[1]) + ")";
		for( unsigned i = 0; i<n_data_dims; i++)
			indices_value += "+(ii" + std::to_string(i) + "*" + std::to_string(size_of_dim[i+2]) + ")";

		// Update the max and index value
		INDT_4 << "if( curmax < " << x->cname() << x_idx << ") {" <<std::endl;
		INDT_4 << "curmax = MAX( curmax, " << x->cname() << x_idx << ");" <<std::endl;
		if( Indices->name != "" )
			INDT_4  << "curmaxind = " << indices_value << ";" <<std::endl;
		INDT_4 << "}" << std::endl;

	}

	virtual void print_output_cell_finalize(std::ostream &dst, const std::string &y_idx) const override
	{
		// Store the calculated values into output tensors
		INDT_3  << y->cname() << y_idx << "= curmax;" << std::endl;
		if( Indices->name != "" )
			INDT_3 << Indices->cname() << y_idx << "= curmaxind;" << std::endl;
	}


	virtual void print(std::ostream &dst) const override
	{
		print_header_info_comment(dst);
		print_loop_with_padding_checks(dst);
	}

	virtual void resolveOutput(const std::vector< const Tensor*> &inputs, std::vector<Tensor *> &outputs) override
	{
		x = inputs[0];

		resolve_strides();
		resolve_dilations();
		resolve_pads();
		resolve_kernel_shape();

		if( storage_order != 0 )
			ERROR("Unimplemented: column-major storage_order");

		Tensor *rv = new Tensor;
		rv->data_dim = resolve_output_size();

		rv->data_type = x->data_type;
		y=rv;
		outputs.push_back(rv);

		update_pads();

		// optional indices vector
		Tensor *indices_out = new Tensor;
		indices_out->data_type = onnx::TensorProto_DataType::TensorProto_DataType_INT64;
		indices_out->data_dim = rv->data_dim;
		Indices = indices_out;
		outputs.push_back(indices_out);


	}
};
}

/************************************************************/
/*    NAME:                                                 */
/*    ORGN: MIT                                             */
/*    FILE: arithmetic.h                                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef ARITHMETICH
#define ARITHMETICH

#include <limits>
#include <cstdlib>

#include "goby/acomms/dccl.h" // for Bitset, DCCLTypedFieldCodec
#include "goby/util/sci.h" // for ceil_log2
#include "goby/util/primitive_types.h" // for goby::uint32, etc

// #define ARITHMETIC_TEST

class ArithmeticCodec :
    public goby::acomms::DCCLRepeatedTypedFieldCodec<goby::int32>
{
  public:
    ArithmeticCodec();
    
private:
    //
    // virtual methods from goby::acomms::DCCLRepeatedTypedFieldCodec<goby::int32>
    //

    // Encode this field using an arithmetic coder given the vector of integers in `wire_values`
    // Returns a set of bits represented the encoded field. Bitset is a typedef for boost::dynamic_bitset<unsigned char>
    // You shouldn't need to worry about the details of Bitset.
    // Use ArithmeticCodec::range_to_bits defined for you to convert
    // ** YOU MUST IMPLEMENT THIS METHOD IN arithmetic.cpp **
    goby::acomms::Bitset encode_repeated(const std::vector<goby::int32>& wire_values);


    // Decode this field using the same arithmetic coder used to encode it.
    // Don't worry about the details of Bitset, use ArithmeticCodec::bits_to_range
    // to get a decimal representation of the binary range in `bits`.
    // ** YOU MUST IMPLEMENT THIS METHOD IN arithmetic.cpp **
    std::vector<goby::int32> decode_repeated(goby::acomms::Bitset* bits);


    // the maximum number of *bits* required to encode any field using this codec
    // in our case, this message will be the least probable set of symbols,
    // such as temperature = [11, 12, 13, 14, 15]
    // this maximum size will be upper bounded by: ceil(log_2(1/P)) + 1 where P is the
    // probability of this least probable set of symbols
    // ** YOU MUST IMPLEMENT THIS METHOD IN arithmetic.cpp **
    unsigned max_size_repeated();
    
    // the minimum number of *bits* required to encode any field using this codec
    // in our case, this message will be the most probable set of symbols,
    // that is temperature = [10, 10, 10, 10, 10]
    // ** YOU MUST IMPLEMENT THIS METHOD IN arithmetic.cpp **
    unsigned min_size_repeated();
    
    // validates the (goby.field) parameters for this codec in the .proto file
    // ** this method has been implemented for you **
    void validate();

    // returns the encoded size of this field given these values
    // ** this method has been implemented for you **
    // (inefficiently in this case by actually encoding the field
    // and seeing what the size is)
    unsigned size_repeated(const std::vector<goby::int32>& field_values);

    enum { LARGEST_MAX_REPEAT_SIZE = 5 };
    
    //
    // normal (not inherited) methods
    //
    
    // the maximum number of repeated values (size of the vector). Same as (goby.field).dccl.max_repeat field in the protobuf file.
    // ** this method has been implemented for you **
    goby::int32 max_repeat() { return DCCLFieldCodecBase::dccl_field_options().max_repeat(); }

    // calculate the bits required to minimally and uniquely represent the decimal range
    // bounded by [range.first, range.second)
    // 
    // the return value is given as a binary decimal, where the least significant bit
    // in the bitset (index 0) represents 2^-1, the next most significant bit (index 1) is 2^-2, and so on
    // ** this method has been implemented for you **
    goby::acomms::Bitset range_to_bits(std::pair<double, double> range);

    // converts a Bitset `bits` created by ArithmeticCodec::range_to_bits back into the range [return.first, return.second)
    // uniquely represented by the set of bits. This range is, in general, a proper subset of the
    // original range passed to ArithmeticCodec::range_to_bits.
    // ** this method has been implemented for you **
    std::pair<double, double> bits_to_range(goby::acomms::Bitset bits);
    
  private:
    // put any data variables you need here...
    
};


#endif

/************************************************************/
/*    NAME:                                                 */
/*    ORGN: MIT                                             */
/*    FILE: arithmetic.cpp                                  */
/*    DATE:                                                 */
/************************************************************/

#include "arithmetic.h"
#include "goby/util/as.h" // for goby::util::as<>()

//
// METHODS TO BE IMPLEMENTED (see arithmetic.h), TESTED (using ctd_tester), AND TURNED IN (see lab assignment)
//

ArithmeticCodec::ArithmeticCodec()
{
}


goby::acomms::Bitset ArithmeticCodec::encode_repeated(const std::vector<goby::int32>& wire_values)
{
    // PLACEHOLDER - returns bitset representing range from [0, 1)
    double lower = 0, upper = 1;    

    // you'll want to find the decimal range to encode the symbols given in `wire_values`
    // ...

    // Bitset data_bits = range_to_bits(std::make_pair(lower, upper));
    // return data_bits;


    // an example for use in decode_repeated
    std::bitset<6> example(std::string("101111"));
    goby::acomms::Bitset example_bits(example.size(), example.to_ulong());
    std::cout << "=== ARITHMETIC DEBUG: \"Encoded\" using: " << example_bits << std::endl;
    return example_bits;
}
    
std::vector<goby::int32> ArithmeticCodec::decode_repeated(goby::acomms::Bitset* bits)
{
    std::cout << "=== ARITHMETIC DEBUG: Starting with " << min_size_repeated() << " bits: " << *bits << std::endl;

    // asking for the remaining 4 bits I encoded with
    bits->get_more_bits(4);
    std::cout << "=== ARITHMETIC DEBUG: All the bits: " << *bits << std::endl;
    
    
    std::vector<goby::int32> out;    
    // std::pair<double, double> result = bits_to_range(bits);
    // PLACEHOLDER - figure out your symbols and add them (push_back) to `out`
    // ...

    
    return out;
}

unsigned ArithmeticCodec::min_size_repeated()
{
    // PLACEHOLDER - change to computed value
    return 2;
}


unsigned ArithmeticCodec::max_size_repeated()
{
    // PLACEHOLDER - change to computed value
    return 6;
}
    
//
// Methods already implemented for you ... be sure to understand what they do (see also arithmetic.h)
//


unsigned ArithmeticCodec::size_repeated(const std::vector<goby::int32>& field_values)
{
    // calculates size using encode_repeated. This is inefficient, but will suffice
    // for this lab
    return encode_repeated(field_values).size();
}        

std::pair<double, double> ArithmeticCodec::bits_to_range(goby::acomms::Bitset bits)
{
    // lower bound is the given bitset followed by 000000...
    double lower = 0;
    for(int i = 0, n = bits.size(); i < n; ++i)
        lower += bits[i]/(pow(2.0, i+1));

    // upper bound is the given bitset followed by 111111...
    // remember 0.111111... = 1 in binary, like 0.999999... = 1 in decimal
    double upper = lower + 1/(pow(2.0, bits.size()));
        
    return std::make_pair(lower,upper);
}

goby::acomms::Bitset ArithmeticCodec::range_to_bits(std::pair<double, double> range)
{
    double lower = range.first, upper = range.second;
    goby::acomms::Bitset lower_bits, upper_bits;

    // keep adding bits until the binary representation of lower and upper
    // differ OR we have found an exact representation for both decimals
    while(lower_bits == upper_bits && !(lower == 0 && upper == 0))
    {
        double lower_int_part, upper_int_part;
        lower *= 2;
        lower = modf(lower, &lower_int_part);
        lower_bits.push_back(lower_int_part >= 1 ? 1 : 0);
            
        upper *= 2;
        upper = modf(upper, &upper_int_part);

        if(upper_int_part == 2) // deal with 1 case
            upper = 1;
        
        upper_bits.push_back(upper_int_part >= 1 ? 1 : 0);
    }

    // if lower is an exact representation return it
    if(lower == 0)
    {
        return lower_bits;
    }
    else // keep adding bits until we uniquely identify the range
    {
        while(bits_to_range(upper_bits).second > range.second)
            upper_bits.push_back(0);

        while(bits_to_range(lower_bits).first < range.first)
            lower_bits.push_back(1);

        // return the smaller of the two representations of the range
        return (lower_bits.size() < upper_bits.size()) ? lower_bits : upper_bits;
    }
        
}

void ArithmeticCodec::validate()
{
    DCCLFieldCodecBase::require(DCCLFieldCodecBase::dccl_field_options().max_repeat() <= LARGEST_MAX_REPEAT_SIZE,
                                "(goby.field).dccl.max_repeat must be less than or equal to " +
                                goby::util::as<std::string>(static_cast<int>(LARGEST_MAX_REPEAT_SIZE)));
}

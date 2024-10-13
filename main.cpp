#include <iostream>
#include <print>
#include <optional>
#include <bit>
#include <type_traits>
#include <string>
#include "decimal.hpp"

using uchar = unsigned char;

constexpr uint8_t MANTISSA_LEN = 23;
constexpr char PRINT_TP[] = "  {}";
constexpr char SUB_TP[] = "- {} (2^{})";
constexpr char ALT_SUB_TP[] = "@ {} (2^{}) ... {}";
constexpr char DIV_TP[] = "{:->{}}";
constexpr size_t TP_BUF_LEN = 3;
constexpr char REP_TP[] = "{:b} {:0>8b} {:0>23b}";

void print_div(const decimal& dec) {
    std::println(DIV_TP, "", dec.length() + TP_BUF_LEN);
}

void convert(const std::string& raw) {
    bool neg = raw.front() == '-';
    decimal dec{{ranges::begin(raw) + neg, ranges::end(raw)}};
    std::println("Sign: {:b}", neg);
    std::optional<uint8_t> exp;
    uint32_t mantissa = 0;
    uint8_t mantissa_d = 0;
    std::optional<decimal> min_alt_err;
    uint32_t mantissa_alt = 0;
    auto sub = [&dec](const decimal& m, auto i) {
        dec.match(m);
        std::println(PRINT_TP, dec);
        std::println(SUB_TP, m, pow_2_max_exp_raw - i);
        print_div(dec);
        dec -= m;
    };
    for(std::make_signed_t<size_t> i = 0; (i < pow_2.size()) && (mantissa_d < MANTISSA_LEN); ++i) {
        if(!dec) {
            mantissa <<= (MANTISSA_LEN - mantissa_d);
            break;
        }
        const auto& pow = pow_2[i];
        if(exp) {
            mantissa <<= 1;
            mantissa_alt <<= 1;
            ++mantissa_d;
            if(dec >= pow) {
                mantissa += 1;
                sub(pow, i);
            } else {
                decimal alt_err = pow;
                alt_err -= dec;
                std::println(PRINT_TP, dec);
                std::println(ALT_SUB_TP, pow, pow_2_max_exp_raw - i, alt_err);
                print_div(dec);
                if((!min_alt_err) || (alt_err < min_alt_err)) {
                    min_alt_err = alt_err;
                    mantissa_alt = mantissa + 1;
                }
            }
        } else if(dec >= pow) {
            if(pow_2_max_exp > i) {
                exp = pow_2_max_exp - i;
            } else {
                exp = 0;
                mantissa += 1;
            }
            sub(pow, i);
        }
    }
    auto final_exp = exp.value_or(0);
    std::println(PRINT_TP, dec);
    std::print("Round to 0: ");
    std::println(REP_TP, neg, final_exp, mantissa);
    std::println("Error: {}", dec);
    
    if(min_alt_err && (min_alt_err < dec)) {
        std::print("Nearest: ");
        std::println(REP_TP, neg, final_exp, mantissa_alt);
        std::println("Error: {}", *min_alt_err);
    }

    float native;
    std::from_chars(raw.data(), raw.data() + raw.length(), native);
    std::println("Native: {:0>32b}", std::bit_cast<uint32_t>(native));
}

void exec(const std::string& raw) {
    if(raw == "exit") {
        std::exit(0);
    } else if(raw == "help") {
        std::println(
            "This tool converts a decimal number (in base 10) to its IEEE-754 representation.\n"
            "Only \".\" is accepted as the decimal separator.\n"
            "Negative numbers should be denoted by a \"-\" prefix. No prefix should be added for a positive number.\n"
            "No other symbols are allowed in the input.\n"
            "Input \"exit\" to exit the program."
        );
        return;
    }
    try {
        convert(raw);
    } catch(const std::runtime_error& e) {
        std::cout << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::string raw;
    if(argc > 1) {
        if(argc > 2) {
            std::println("Only one argument is allowed!");
            return 1;
        } else {
            raw = argv[1];
            exec(raw);
            return 0;
        }
    } else {
        std::println(
            "IEEE754 Converter\n"
            "Type \"help\" for more information on how to use the tool."
        );
    }
    while(true) {
        std::print(">> ");
        std::getline(std::cin, raw);
        exec(raw);
    }
}
#pragma once

// use DYNLEC_CT_ENCRYPT to encrypt strings at compile time
#define DYNLEC_CT_ENCRYPT(input) [] { \
        static char buffer[sizeof(input)]; \
        constexpr ::Dynlec::CTEncrypt<sizeof(input), __COUNTER__> cte(input); \
        return cte.decryptTo(buffer); \
    }();

// use DYNLEC_CT_ENCRYPT_INPLACE for members and variables initialized
// before being used to enable lazy decryption. obtain string from def
// using .obtain method
#define DYNLEC_CT_ENCRYPT_INPLACE(def, input) \
    ::Dynlec::CTEBuffer<sizeof(input)> def { \
        [] { \
            constexpr ::Dynlec::CTEncrypt<sizeof(input)> cte{ input }; \
            return cte; \
        }() \
    }

#ifndef DYNLEC_CT_ENCRYPT_SEED
constexpr char Dynlec_CT_Encrypt_StaticTime[9] = __TIME__;
#define DYNLEC_CT_ENCRYPT_SEED (\
    ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[7] - '0') * 1ull    + ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[6] - '0') * 10ull  + \
    ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[4] - '0') * 60ull   + ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[3] - '0') * 600ull + \
    ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[1] - '0') * 3600ull + ((unsigned long long) Dynlec_CT_Encrypt_StaticTime[0] - '0') * 36000ull)
#endif

#ifndef DYNLEC_CT_ENCRYPT_ROUNDS
#define DYNLEC_CT_ENCRYPT_ROUNDS 10
#endif

namespace Dynlec
{
    template <unsigned size, unsigned counter = 0>
    class CTEncrypt
    {
    public:
        inline constexpr CTEncrypt(const char* string)
        {
            for (unsigned long long i = 0u; i < size; ++i)
                input[i] = string[i] ^ random_character(i);
        }

        CTEncrypt(const CTEncrypt<size, counter>& cte)
        {
            for (int i = 0; i < size; ++i)
                input[i] = cte.input[i];
        }

        char* decryptTo(char(&buffer)[size]) const
        {
            for (unsigned long long i = 0; i < length; i++) {
                buffer[i] = input[i] ^ random_character(i);
            }
            buffer[length] = '\0';
            return buffer;
        }

    private:
        const unsigned length = (size - 1);
        char input[size] = { };

        constexpr char random_character(signed long long index) const
        {
            return static_cast<char>(linear_congruent_generator(
                DYNLEC_CT_ENCRYPT_ROUNDS,
                DYNLEC_CT_ENCRYPT_SEED + size * index + counter) % (0xFF + 1));
        }

        constexpr unsigned long long linear_congruent_generator(
            unsigned long long rounds,
            unsigned long long seed) const
        {
            return 1013904223ull + (1664525ull * (
                (rounds > 0)
                    ? linear_congruent_generator(rounds - 1, seed)
                    : (seed))
                ) % 0xFFFFFFFF;
        }
    };

    template <unsigned size, unsigned counter = 0>
    class CTEBuffer
    {
    public:
        CTEBuffer(const CTEncrypt<size, counter>& cte)
            :
            cte(cte)
        {
        }

        const char* obtain()
        {
            if (processed == false)
            {
                processed = true;
                cte.decryptTo(result);
            }

            return result;
        }

    private:
        bool processed = false;
        char result[size];
        const CTEncrypt<size, counter> cte;
    };
}

static_assert(sizeof(char) == 1, "DynlecEncrypt is made for character size of 1");

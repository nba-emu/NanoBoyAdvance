#pragma once

namespace util
{
    namespace bit
    {
        /// Extracts a single bit from a value.
        /// @tparam   T         value's type
        /// @tparam   position  bit position
        /// @param    value     value to get the bit from
        /// @returns  the extracted bit.
        template <typename T, int position>
        int get(T value)
        {
            return (value >> position) & 1;
        }

        /// Extracts a single bit from a value, no final shift applied.
        /// @tparam   T         value's type
        /// @tparam   position  bit position
        /// @param    value     value to get the bit from
        /// @returns  the extracted bit.
        template <typename T, int position>
        int get_flag(T value)
        {
            return value & (1 << position);
        }

        /// Sets the value of a single bit.
        /// @tparam  T         value's type
        /// @tparam  position  bit position
        /// @param   value     value to get the bit from
        /// @param   state     new value of the bit (0 = false, 1 = true)
        template <typename T, int position>
        void set(T& value, bool state)
        {
            value = (value & ~(1 << position)) | (state ? (1 << position) : 0);
        }

        /// Toggles the value of a single bit.
        /// @tparam  T         value's type
        /// @tparam  position  bit position
        template <typename T, int position>
        void toggle(T& value)
        {
            value ^= 1 << position;
        }
    }

    namespace field
    {
        /// Extracts a bitfield from a value.
        /// @tparam   T         value's type
        /// @tparam   position  field position (lowest bit)
        /// @tparam   length    field length (in bits)
        /// @param    value     source value
        /// @returns  the extracted bits.
        template <typename T, int position, int length>
        int get(T value)
        {
            T mask = 0;

            for (int i = 0; i < length; i++) mask = (mask << 1) | 1;

            return (value >> position) & mask;
        }

        /// Updates a bitfield in a given value.
        /// @tparam  T         value's type
        /// @tparam  position  field position (lowest bit)
        /// @tparam  length    field length (in bits)
        /// @param   value     value to manipulate
        /// @param   field_value  new value of the field
        template <typename T, int position, int length>
        void set(T& value, T field_value)
        {
            T mask = 0;

            for (int i = 0; i < length; i++) mask = (mask << 1) | 1;

            value = (value & ~(mask << position)) | ((field_value & mask) << position);
        }
    }
}

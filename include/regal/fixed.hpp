#pragma once

#include <cstdint>
#include <type_traits>

namespace regal
{
	template <typename IntegralType, typename PromotedIntegralType, std::size_t Precision>
	class fixed
	{
	private:
		template<typename, typename>
		static constexpr bool shared_template = false;

		template
		<
			template<typename, typename, std::size_t> class Template,

			typename A_IT, typename A_PIT, std::size_t A_P,
			typename B_IT, typename B_PIT, std::size_t B_P
		>
		static constexpr bool shared_template
		<
			Template<A_IT, A_PIT, A_P>,
			Template<B_IT, B_PIT, B_P>
		> = true;
	protected:
		template <typename raw_value_t, typename = std::enable_if<std::is_integral_v<raw_value_t>>>
		static constexpr raw_value_t offset_by_precision_space(raw_value_t value)
		{
			return (value << Precision);
		}

		template <typename raw_value_t, typename = std::enable_if<std::is_integral_v<raw_value_t>>>
		static constexpr raw_value_t truncate_precision_space(const raw_value_t& value)
		{
			return (value >> Precision);
		}

		static constexpr IntegralType precision_mask = offset_by_precision_space(static_cast<IntegralType>(1));

		template <typename fp_t>
		static constexpr fp_t precision_mask_fp()
		{
			return static_cast<fp_t>(precision_mask);
		}

		// Internal construction from raw value input.
		template <typename int_t, typename=std::enable_if<std::is_integral_v<int_t>>>
		static constexpr fixed from_raw_value(int_t value)
		{
			fixed f;

			f.value = static_cast<IntegralType>(value);

			return f;
		}

		template <typename T, typename ShiftType>
		static constexpr T dynamic_shift(T value, ShiftType left_influence, ShiftType right_influence)
		{
			if (left_influence > right_influence) // -->
			{
				return (value >> (left_influence - right_influence));
			}
			else if (right_influence > left_influence) // <--
			{
				return (value << (right_influence - left_influence));
			}
			//else { /* Do nothing. */ }

			return value;
		}

		IntegralType value = IntegralType(0); // <-- Explicit zero-out. (May change this later)

		// Same as `get_value`, but promotes from `IntegralType` to `PromotedIntegralType`. (Does not perform precision adjustment)
		constexpr PromotedIntegralType get_large_value() const
		{
			return static_cast<PromotedIntegralType>(value);
		}
	public:
		//using fp_t = double;
		using value_type = IntegralType;
		using promoted_type = PromotedIntegralType;
		using size_t = std::size_t;

		constexpr fixed() = default;

		template <typename T>
		constexpr fixed(T value)
		{
			if constexpr (shared_template<T, fixed>)
			{
				*this = value;
			}
			else if (std::is_floating_point_v<T>)
			{
				this->value = static_cast<IntegralType>(value * static_cast<T>(precision_mask) + (value >= 0 ? 0.5 : -0.5)); // >
			}
			else if (std::is_integral_v<T>)
			{
				this->value = offset_by_precision_space(static_cast<IntegralType>(value));
			}
			else
			{
				static_assert("Unable to determine correct type conversion.");
			}
		}

		constexpr const IntegralType& get_value() const // IntegralType
		{
			return this->value;
		}

		constexpr size_t get_precision() const
		{
			return Precision;
		}

		constexpr IntegralType get_precision_mask() const
		{
			return precision_mask;
		}

		template <typename T>
		constexpr operator T() const
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				return (static_cast<T>(this->value) / precision_mask_fp<T>());
			}

			if constexpr (std::is_integral_v<T>)
			{
				return truncate_precision_space(static_cast<T>(this->value));
			}

			return *this;
		}

		template <typename T>
		constexpr fixed& operator=(const T& value)
		{
			// Where `T` is a differently configured `fixed` type.
			if constexpr (shared_template<T, fixed>)
			{
				auto mask = (value.get_precision_mask() - 1);
				auto precision_bits = (value.get_value() & mask);

				precision_bits = dynamic_shift(precision_bits, value.get_precision(), this->get_precision());

				this->value = (offset_by_precision_space(static_cast<IntegralType>(value)) | precision_bits);

				return *this;
			}

			// All other scenarios (see same-format overload):
			*this = fixed(value);

			return *this;
		}

		// Same format.
		template <>
		constexpr fixed& operator=<fixed>(const fixed& f)
		{
			this->value = f.value;

			return *this;
		}

		constexpr fixed operator+(const fixed& f) const
		{
			return from_raw_value(f.value + this->value);
		}

		constexpr fixed& operator+=(const fixed& f)
		{
			this->value += f.value;

			return *this;
		}

		constexpr fixed operator-(const fixed& f) const
		{
			return from_raw_value(f.value - this->value);
		}

		constexpr fixed& operator-=(const fixed& f)
		{
			this->value -= f.value;

			return *this;
		}

		constexpr fixed operator-() const
		{
			return from_raw_value(-this->value);
		}

		constexpr fixed operator*(const fixed& f) const
		{
			// Convert `this->value` and `f.value` into `PromotedIntegralType`'s working space, then truncate.
			return from_raw_value((truncate_precision_space(this->get_large_value() * f.get_large_value())));
		}

		constexpr fixed& operator*=(const fixed& f)
		{
			//return *this = this->operator*(f);
		
			// Same as `operator*`, but we assign `value` inplace, instead of constructing a new object.
			this->value = static_cast<IntegralType>(truncate_precision_space(this->get_large_value() * f.get_large_value()));

			return *this;
		}

		constexpr fixed operator/(const fixed& f) const
		{
			// Convert `this->value` into `PromotedIntegralType`'s working space, offset by space dedicated to precision, then perform division on `f.value` (also promoted to `PromotedIntegralType`).
			return from_raw_value(offset_by_precision_space(this->get_large_value()) / f.get_large_value());
		}

		constexpr fixed operator/=(const fixed& f)
		{
			//return *this = this->operator/(f);

			// Same as `operator/`, but we assign `value` inplace, instead of constructing a new object.
			this->value = static_cast<IntegralType>(offset_by_precision_space(this->get_large_value()) / f.get_large_value());

			return *this;
		}

		constexpr auto operator<=>(const fixed& f) const
		{
			return (this->value <=> f.value);
		}

		// Forward declaration of friend stream output operator. (see below)
		template <typename IntegralType, typename PromotedIntegralType, std::size_t Precision>
		friend constexpr std::ostream& operator<<(std::ostream& os, const fixed<IntegralType, PromotedIntegralType, Precision>& f);
	};

	using fp4_4   = fixed<std::int8_t, int16_t, 4>;
	using fp8_8   = fixed<std::int16_t, int32_t, 8>;
	using fp16_16 = fixed<std::int32_t, std::int64_t, 16>;
	using fp24_8  = fixed<std::int32_t, std::int64_t, 8>;

	fp4_4   constexpr operator ""_fp4_4(long double f)   { return f; }
	fp8_8   constexpr operator ""_fp8_8(long double f)   { return f; }
	fp16_16 constexpr operator ""_fp16_16(long double f) { return f; }
	fp24_8  constexpr operator ""_fp24_8(long double f)  { return f; }

	template <typename IntegralType, typename PromotedIntegralType, std::size_t Precision>
	constexpr std::ostream& operator<<(std::ostream& os, const fixed<IntegralType, PromotedIntegralType, Precision>& f)
	{
		return os << static_cast<double>(f); // long double
	}
}
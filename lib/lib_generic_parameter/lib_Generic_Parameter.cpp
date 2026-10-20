#include <cassert>
#include <cstring>

#include "lib_Generic_Parameter.h"

static uint8_t lib_Generic_Parameter_ValueSizeWithoutType(
	lib_Generic_Parameter_Value_t value)
{
	switch (value.type)
	{
		// void is carried as a bool: the device firmware encodes void as a single bool byte
		// we have to reserve, compare and serialize that byte just like an explicit bool
		case lib_Generic_Parameter_Type_void:
		case lib_Generic_Parameter_Type_bool:
		case lib_Generic_Parameter_Type_ProcedureCall:
			return sizeof(value.b);

		case lib_Generic_Parameter_Type_char:
			return sizeof(value.ch);

		case lib_Generic_Parameter_Type_s8:
			return sizeof(value.s8);

		case lib_Generic_Parameter_Type_u8:
			return sizeof(value.u8);

		case lib_Generic_Parameter_Type_s16LE:
		case lib_Generic_Parameter_Type_s16BE:
			return sizeof(value.s16);

		case lib_Generic_Parameter_Type_u16LE:
		case lib_Generic_Parameter_Type_u16BE:
			return sizeof(value.u16);

		case lib_Generic_Parameter_Type_s32LE:
		case lib_Generic_Parameter_Type_s32BE:
			return sizeof(value.s32);

		case lib_Generic_Parameter_Type_u32LE:
		case lib_Generic_Parameter_Type_u32BE:
			return sizeof(value.u32);

		case lib_Generic_Parameter_Type_s64LE:
		case lib_Generic_Parameter_Type_s64BE:
			return sizeof(value.s64);

		case lib_Generic_Parameter_Type_u64LE:
		case lib_Generic_Parameter_Type_u64BE:
			return sizeof(value.u64);

		case lib_Generic_Parameter_Type_f32LE:
		case lib_Generic_Parameter_Type_f32BE:
			return sizeof(value.f32);

		case lib_Generic_Parameter_Type_f64LE:
		case lib_Generic_Parameter_Type_f64BE:
			return sizeof(value.f64);

		default:
			return 0;
	}
}

bool lib_Generic_Parameter_ValueIsEqual(
	lib_Generic_Parameter_Value_t v1,
	lib_Generic_Parameter_Value_t v2)
{
	if (v1.type != v2.type)
	{
		return false;
	}

	switch (v1.type)
	{
		// void is compared as a bool (see size function): the trailing byte is
		// significant, so do not treat all void values as equal.
		case lib_Generic_Parameter_Type_void:
		case lib_Generic_Parameter_Type_bool:
		case lib_Generic_Parameter_Type_ProcedureCall:
			return v1.b == v2.b;

		case lib_Generic_Parameter_Type_char:
			return v1.ch == v2.ch;

		case lib_Generic_Parameter_Type_s8:
			return v1.s8 == v2.s8;

		case lib_Generic_Parameter_Type_u8:
			return v1.u8 == v2.u8;

		case lib_Generic_Parameter_Type_s16LE:
		case lib_Generic_Parameter_Type_s16BE:
			return v1.s16 == v2.s16;

		case lib_Generic_Parameter_Type_u16LE:
		case lib_Generic_Parameter_Type_u16BE:
			return v1.u16 == v2.u16;

		case lib_Generic_Parameter_Type_s32LE:
		case lib_Generic_Parameter_Type_s32BE:
			return v1.s32 == v2.s32;

		case lib_Generic_Parameter_Type_u32LE:
		case lib_Generic_Parameter_Type_u32BE:
			return v1.u32 == v2.u32;

		case lib_Generic_Parameter_Type_s64LE:
		case lib_Generic_Parameter_Type_s64BE:
			return v1.s64 == v2.s64;

		case lib_Generic_Parameter_Type_u64LE:
		case lib_Generic_Parameter_Type_u64BE:
			return v1.u64 == v2.u64;

		case lib_Generic_Parameter_Type_f32LE:
		case lib_Generic_Parameter_Type_f32BE:
			return v1.f32 == v2.f32;

		case lib_Generic_Parameter_Type_f64LE:
		case lib_Generic_Parameter_Type_f64BE:
			return v1.f64 == v2.f64;

		default:
			return false;
	}
}

uint8_t lib_Generic_Parameter_SizeWithType(
	lib_Generic_Parameter_Value_t value)
{
	return static_cast<uint8_t>(
		sizeof(value.type) +
		lib_Generic_Parameter_ValueSizeWithoutType(value));
}

uint8_t lib_Generic_Parameter_SerializeValueAndType(
	lib_Generic_Parameter_Value_t value,
	void *pDest,
	uint8_t maxLength)
{
	assert(pDest != nullptr);

	const uint8_t valueSize =
		lib_Generic_Parameter_ValueSizeWithoutType(value);

	const uint8_t totalSize =
		static_cast<uint8_t>(sizeof(value.type) + valueSize);

	if (valueSize == 0)
	{
		return 0;
	}

	if (totalSize > maxLength)
	{
		return 0;
	}

	uint8_t *dest = static_cast<uint8_t *>(pDest);

	std::memcpy(dest, &value.type, sizeof(value.type));
	dest += sizeof(value.type);

	switch (value.type)
	{
		// void is serialized as a bool: size function reserves a byte for it, so
		// we must actually copy value.b (the old code reported the size but left
		// the byte uninitialized).
		case lib_Generic_Parameter_Type_void:
		case lib_Generic_Parameter_Type_bool:
		case lib_Generic_Parameter_Type_ProcedureCall:
			std::memcpy(dest, &value.b, valueSize);
			break;

		case lib_Generic_Parameter_Type_char:
			std::memcpy(dest, &value.ch, valueSize);
			break;

		case lib_Generic_Parameter_Type_s8:
			std::memcpy(dest, &value.s8, valueSize);
			break;

		case lib_Generic_Parameter_Type_u8:
			std::memcpy(dest, &value.u8, valueSize);
			break;

		case lib_Generic_Parameter_Type_s16LE:
		case lib_Generic_Parameter_Type_s16BE:
			std::memcpy(dest, &value.s16, valueSize);
			break;

		case lib_Generic_Parameter_Type_u16LE:
		case lib_Generic_Parameter_Type_u16BE:
			std::memcpy(dest, &value.u16, valueSize);
			break;

		case lib_Generic_Parameter_Type_s32LE:
		case lib_Generic_Parameter_Type_s32BE:
			std::memcpy(dest, &value.s32, valueSize);
			break;

		case lib_Generic_Parameter_Type_u32LE:
		case lib_Generic_Parameter_Type_u32BE:
			std::memcpy(dest, &value.u32, valueSize);
			break;

		case lib_Generic_Parameter_Type_s64LE:
		case lib_Generic_Parameter_Type_s64BE:
			std::memcpy(dest, &value.s64, valueSize);
			break;

		case lib_Generic_Parameter_Type_u64LE:
		case lib_Generic_Parameter_Type_u64BE:
			std::memcpy(dest, &value.u64, valueSize);
			break;

		case lib_Generic_Parameter_Type_f32LE:
		case lib_Generic_Parameter_Type_f32BE:
			std::memcpy(dest, &value.f32, valueSize);
			break;

		case lib_Generic_Parameter_Type_f64LE:
		case lib_Generic_Parameter_Type_f64BE:
			std::memcpy(dest, &value.f64, valueSize);
			break;

		default:
			return 0;
	}

	return totalSize;
}
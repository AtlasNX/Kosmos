#include "encoding.hpp"

std::vector<u8> Encoding::uft16leToUtf8(u8 *data, size_t length) {
  std::vector<u16> utf16Chars(length / 2);
	std::vector<u8> utf8Chars;
	utf8Chars.reserve(length / 2);

	for (u32 i = 0; i < length; i += 2) {
		u16 c = data[i] | data[i + 1] << 8;
		utf16Chars[i / 2] = c;
	}

	for (u32 character = 0; character < length / 2; character++) {
		u16 currChar = utf16Chars[character];

		if (currChar <= 0x007F) {
			utf8Chars.push_back(currChar & 0xFF);
		}
		else if (currChar <= 0x07FF) {
			utf8Chars.push_back(0xC0 | ((currChar & 0x7C0) >> 6));
			utf8Chars.push_back(0x80 | (currChar & 0x3F));
		}
		else {
			utf8Chars.push_back(0xE0 | ((currChar & 0xF000) >> 12));
			utf8Chars.push_back(0x80 | ((currChar & 0xFC0) >> 6));
			utf8Chars.push_back(0x80 | (currChar & 0x3F));
		}
	}

	utf8Chars.push_back(0x00);
	utf8Chars.shrink_to_fit();

	return utf8Chars;
}

std::vector<u8> Encoding::uft16beToUtf8(u8 *data, size_t length) {
  std::vector<u16> utf16Chars(length / 2);
	std::vector<u8> utf8Chars;
	utf8Chars.reserve(length / 2);

	for (u32 i = 0; i < length; i += 2) {
		u16 c = data[i + 1] | data[i] << 8;
		utf16Chars[i / 2] = c;
	}

	for (u32 character = 0; character < length / 2; character++) {
		u16 currChar = utf16Chars[character];

		if (currChar <= 0x007F) {
			utf8Chars.push_back(currChar & 0xFF);
		}
		else if (currChar <= 0x07FF) {
			utf8Chars.push_back(0xC0 | ((currChar & 0x7C0) >> 6));
			utf8Chars.push_back(0x80 | (currChar & 0x3F));
		}
		else {
			utf8Chars.push_back(0xE0 | ((currChar & 0xF000) >> 12));
			utf8Chars.push_back(0x80 | ((currChar & 0xFC0) >> 6));
			utf8Chars.push_back(0x80 | (currChar & 0x3F));
		}
	}

	utf8Chars.push_back(0x00);
	utf8Chars.shrink_to_fit();

	return utf8Chars;
}

std::vector<u8> Encoding::utf8ToUtf16le(u8 *data, size_t length) {
  std::vector<u8> utf16Chars;
	utf16Chars.reserve(length / 2);

	for (u32 character = 0; character < length;) {
		if ((data[character] & 0x80) == 0x00) {
			utf16Chars.push_back(data[character]);
			utf16Chars.push_back(0x00);
			character++;
		}
		else if ((data[character] & 0xE0) == 0xC0) {
			u16 decodedCharacter = (data[character + 1] & 0x3F) | ((data[character] & 0x1F) << 6);
			utf16Chars.push_back(decodedCharacter & 0xFF);
			utf16Chars.push_back(decodedCharacter >> 8);
			character += 2;
		}
		else if ((data[character] & 0xF0) == 0xE0) {
			u16 decodedCharacter = (data[character + 2] & 0x3F) | (data[character + 1] & 0x3F) << 6 | ((data[character] & 0x0F) << 12);
			utf16Chars.push_back(decodedCharacter & 0xFF);
			utf16Chars.push_back(decodedCharacter >> 8);
			character += 3;
		}
	}

	utf16Chars.shrink_to_fit();

	return utf16Chars;
}

std::vector<u8> Encoding::utf8ToUtf16be(u8 *data, size_t length) {
  std::vector<u8> utf16Chars;
	utf16Chars.reserve(length / 2);

	for (u32 character = 0; character < length;) {
		if ((data[character] & 0x80) == 0x00) {
			utf16Chars.push_back(0x00);
			utf16Chars.push_back(data[character]);
			character++;
		}
		else if ((data[character] & 0xE0) == 0xC0) {
			u16 decodedCharacter = (data[character + 1] & 0x3F) | ((data[character] & 0x1F) << 6);
			utf16Chars.push_back(decodedCharacter >> 8);
			utf16Chars.push_back(decodedCharacter & 0xFF);
			character += 2;
		}
		else if ((data[character] & 0xF0) == 0xE0) {
			u16 decodedCharacter = (data[character + 2] & 0x3F) | (data[character + 1] & 0x3F) << 6 | ((data[character] & 0x0F) << 12);
			utf16Chars.push_back(decodedCharacter >> 8);
			utf16Chars.push_back(decodedCharacter & 0xFF);
			character += 3;
		}
	}

	utf16Chars.shrink_to_fit();

	return utf16Chars;
}

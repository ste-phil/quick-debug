#ifndef QD_STRFORMAT_HPP
#define QD_STRFORMAT_HPP

namespace QD::StrFormat 
{
	std::string PadR(const std::string& text, const int padding)
	{
		assert(text.length() <= padding);
		auto insertPadding = padding - text.length();

		std::string res;
		res.resize(padding);

		for (size_t i = 0; i < insertPadding; i++) {
			res[i] = ' ';
		}
		for (size_t i = insertPadding; i < padding; i++) {
			res[i] = text[i - insertPadding];
		}

		return res;
	}

	std::string PadL(const std::string& text, const int padding)
	{
		assert(text.length() <= padding);

		std::string res;
		res.resize(padding);

		for (size_t i = 0; i < text.length(); i++) {
			res[i] = text[i];
		}
		for (size_t i = text.length(); i < padding; i++) {
			res[i] = ' ';
		}

		return res;
	}
}

#endif
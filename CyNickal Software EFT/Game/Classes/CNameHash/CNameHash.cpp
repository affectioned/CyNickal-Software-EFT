#include "pch.h"

#include "CNameHash.h"

std::unordered_map<uint32_t, std::string> m_WeaponNames
{
	{0x0BFA7DBD, "Saiga-12k"},
	{0x032142AA, "TOZ KS-23M"},
	{0xAE4D2212, "TOZ-106"},
	{0x68103A98, "MP-155"},
	{0xED31C50B, "MP-153"},
	{0xA10D4A9F, "MP-133"},
	{0x3A9D66E8, "Vepr"},
	{0xF89323EC, "Vepr-Hunter"},
	{0x2CA90C36, "Camper Axe"},
	{0xC3E4DC81, "AKS-74U"},
	{0x61961C9F, "Saiga-9"},
	{0x1053DA5A, "Saiga-12 FA"},
	{0xC95E7F32, "VPO-209"},
	{0x905A34FF, "PM"},
	{0x065CEB7C, "TT"},
	{0xE8BE68AA, "Desert Eagle"},
	{0x581508B6, "AK-74N"},
	{0x6CBC707F, "MP9"},
	{0x73B9DCD9, "SKS"},
	{0xD9C5BAD4, "MP-443 Grach"},
	{0xEAA0376F, "RPK-16"},
	{0x2055395E, "OP-SKS"},
	{0x73EE1B07, "AK-74M"},
	{0xCF7C4C8F, "AK-105"},
	{0xC7D1A827, "ADAR 2"},
	{0xAC676528, "Salewa"},
	{0xA967B8A6, "PP-91"},
	{0x382F41EC, "590A1"},
	{0x8A20059A, "Milk"},
	{0x2110A498, "MP-443"},
	{0xEFE162B5, "AK-104"},
	{0x095614E9, "M1A"},
	{0x88A5F812, "NL545"},
	{0xF1B44B48, "NL545"},
	{0xB2F3D739, "MP9-N"},
	{0xC4C341F6, "AK-545"},
	{0x5D2895D3, "Mosin Infantry"},
	{0x3FA97F0F, "Mosin Infantry"},
	{0x463C2B8C, "Mosin"},
	{0xF31A4204, "AKMS"},
	{0xAED7C176, "PKTM"},
	{0x89D12ECF, "Aquamari"},
	{0x975C3D06, "M67"},
	{0x55D5BA0F, "Splint+"},
	{0x9c5b7355, "Splint"},
	{0xde317059, "Grizzly"},
	{0x7CCD75F2, "MXLR"},
	{0x98AFA2DC, "AKS-74"},
	{0x98879D3B, "SVD"},
	{0x640DFFA2, "SR-25"},
	{0xAEF352D8, "HK G36"},
	{0xAE1830F2, "AK-545 Short"},
	{0x7AEE21CD, "SV-98"},
	{0x90DBD156, "AVT-40"},
	{0xA7A0A474, "Rsh-12"},
	{0xE9FB687D, "RFB"},
	{0xA64E7E41, "AA-12"},
	{0x4AC6DB7F, "MCX .300"},
	{0x36F96D5A, "FN-SCAR"},
	{0x1173B972, "RSASS"},
	{0xA3337E07, "AK-103"},
	{0x7DD65260, "IWI UZI"},
	{0x9A6EDF14, "PPSh-41"},
	{0xFB2CD802, "M4A1"},
	{0x71A0A377, "M16A1"},
	{0x529E1F4F, "VOG"},
	{0xA0A73DBA, "AKS-74N"},
	{0x955BAEC7, "KBP 9A-91"},
	{0xE032A35E, "DVL-10" },
	{ 0x3E1A7F15, "Zarya" },
	{ 0x9F3D9DD7, "MP5K" },
	{ 0xAE6E252, "STM-9" },
	{ 0xA9589FB0, "AK-74N" },
	{ 0xCA37451C, "AKS-74UB" },
	{ 0x8FC73C19, "Remington 870" },
	{ 0xBBD3128F, "ASh-12" },
	{ 0x39A305FF, "VPO-136 Vepr" },
	{ 0x462B2E29, "Vepr-Hunter" },
};

std::unordered_map<uint32_t, std::string> m_AmmoNames
{
	{0x923f19a4, "Buckshot"},
	{0x40197F32, "Buckshot"},
	{0x516ED055, "Buckshot"},
	{0x10BEBCC7, "Slug"},
	{0x2b3966c4, "TT P gl"},
	{0x1b69be1e, "BT"},
	{0x8FB566EC, "PM P gzh"},
	{0xDF62E382, "Devastator Slug"},
	{0x7F8CD91, "Poleva-6u Slug"},
	{0xB0790FD, "Piranha"},
	{0x3F7A1CF3, "PSO gzh"},
	{0xD67DC0D4, "M62"},
	{0xF9D453BD, "HP"},
	{0xBF84582A, "7BT1"},
	{0x5E25F12C, "M856"},
	{0x506855B1, "M855"},
	{0x5fce4113, "M995"},
	{0x17F53CF2, "M80"},
	{0x8369C9CB, "M855A1"},
	{0xE111DDB2, "M882"},
	{0x8B6DF6C7, "PS"},
	{0x1BD7BEF2, "BP"},
	{0x85ADCFC2, "Flechette"},
	{0x929E1DD8, "AP 63"},
	{0x9249F823, "SIG FMJ"},
	{0xE7AFFE6E, "Grizzly Slug"},
	{0x70E8F770, "SP"},
	{0x5C2AD700, "Copper Slug"},
	{0xE6EE4A6E, "US"},
	{0x3DCECE9C, "HP"},
	{0xAB77CE43, "FMJ"},
	{0x53B2AF25, "FMJ"},
	{0xC26D8F5F, "Ultra Noiser"},
	{0xC2A6B027, "T46M"},
	{0xF8AC0FDB, "T45M"},
	{0x79524, "TKM FMJ"},
	{0x1D54FD88, "BZT gzh"},
	{0xB5BB06B7, "7N1"},
	{0x366D3F54, "M856A1"},
	{0xE8E19E6E, "TKM Geksa"},
	{0xD649E27C, "GT"},
	{0x73809C75, "TKM EKO"},
	{0x5AAC89AE, "BPZ FMJ"},
	{0xE8683D4E, "TPZ SP"},
};

std::array<std::unordered_map<uint32_t, std::string>, 4> m_NameMaps
{
	m_WeaponNames,
	m_AmmoNames,
};

uint32_t JOAAT(const std::string& String) {
	size_t i = 0;
	uint32_t hash = 0;
	while (i != String.size()) {
		if (String[i] == '\0') break;
		hash += String[i++];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

CNameHash::CNameHash(const std::string& ItemName)
{
	m_Hash = JOAAT(ItemName);

	if (m_Hash)
		std::println("[CItemHash] Generated hash {0:X} for item name {1}", m_Hash, std::string(ItemName.begin(), ItemName.end()).c_str());
}

const std::string* CNameHash::GetSanitizedName(ENameMap map) const
{
	auto UnderlyingMap = std::to_underlying(map);

	auto& NameMap = m_NameMaps[UnderlyingMap];

	if (NameMap.find(m_Hash) != NameMap.end())
		return &NameMap.at(m_Hash);

	return nullptr;
}
#pragma once

enum class ENameMap : uint8_t
{
	Weapons,
	Ammo,
};

class CNameHash
{
public:
	CNameHash(const std::string& WeaponName);
	const uint32_t GetHash() const { return m_Hash; }
	const std::string* GetSanitizedName(ENameMap map) const;

private:
	uint32_t m_Hash{ 0 };
};
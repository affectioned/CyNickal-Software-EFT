#include "pch.h"
#include "Offsets.h"
#include "DMA/DMA.h"
#include "Game/EFT.h"
#include "DMA/Signature Scanner.h"

namespace Offsets
{
	// TODO: change from RVA, but will have to update the base itself in some places

	//	.text:000000018031DB22 48 89 05 77 58 70 01                                mov     cs : qword_181A233A0, rax
	//	.text : 000000018031DB29 48 83 C4 38                                         add     rsp, 38h
	//	.text:000000018031DB2D C3                                                  retn
	//	.text:000000018031DB2E; -------------------------------------------------------------------------- -
	//	.text:000000018031DB2E
	//	.text:000000018031DB2E                                     loc_18031DB2E : ; CODE XREF : sub_18031DAD0 + 2B↑j
	//	.text : 000000018031DB2E 33 C9 xor ecx, ecx
	bool ResolveGOM(DMA_Connection* Conn)
	{
		auto& proc = EFT::GetProcess();
		auto pid = proc.GetPID();

		uintptr_t base = proc.GetUnityAddress();
		size_t    size = proc.GetUnitySize();

		if (!base || !size)
			return false;

		uintptr_t gomPtr = SignatureScanner::FindSignature(
			Conn,
			"48 89 05 ? ? ? ? 48 83 C4 ? C3 33 C9",
			base,
			base + size,
			pid
		);

		uintptr_t readAddr = gomPtr + 3;

		int relative = proc.ReadMem<int>(Conn, readAddr);

		uintptr_t GOM = gomPtr + 7 + relative;

		uintptr_t GOMRVA = GOM - base;

		Offsets::pGOM = GOMRVA;

		std::println("[+] GOM RVA Offset: 0x{:X}", Offsets::pGOM);

		return GOM != 0;
	}

	//	.text:00000001806DE095 4C 89 B4 24 A0 00 00 00                             mov[rsp + 0D8h + var_38], r14
	//	.text:00000001806DE09D 48 8D 0D DC A4 36 01                                lea     rcx, qword_181A48580
	//	.text:00000001806DE0A4 E8 97 4C 01 00                                      call    sub_1806F2D40
	bool ResolveCameras(DMA_Connection* Conn)
	{
		auto& proc = EFT::GetProcess();
		auto pid = proc.GetPID();

		uintptr_t base = proc.GetUnityAddress();
		size_t    size = proc.GetUnitySize();

		if (!base || !size)
			return false;

		uintptr_t camerasPtr = SignatureScanner::FindSignature(
			Conn,
			"48 8B 05 ? ? ? ? 49 C7 C6 ? ? ? ? 8B 48 ? 85 C9 0F 84 ? ? ? ? 48 89 9C 24",
			base,
			base + size,
			pid
		);

		uintptr_t readAddr = camerasPtr + 3;

		int relative = proc.ReadMem<int>(Conn, readAddr);

		uintptr_t Cameras = camerasPtr + 7 + relative;

		uintptr_t CamerasRVA = Cameras - base;

		Offsets::pCameras = CamerasRVA;

		std::println("[+] Cameras RVA Offset: 0x{:X}", Offsets::pCameras);

		return Cameras != 0;
	}


	// il2cpp : 00000001807370F5 48 8D 0D FC 91 0D 05                                lea     rcx, qword_1858102F8
	// il2cpp : 00000001807370FC E8 0F 0E B9 FF                                      call    sub_1802C7F10
	// il2cpp : 0000000180737101 C6 05 C2 17 24 05 01                                mov     cs : byte_1859788CA, 1
	// il2cpp : 0000000180737108
	// il2cpp : 0000000180737108                                     loc_180737108 : ; CODE XREF : sub_180737090 + 63↑j
	// il2cpp : 0000000180737108 48 85 DB                                            test    rbx, rbx
	// il2cpp : 000000018073710B 75 0A                                               jnz     short loc_180737117
	// il2cpp : 000000018073710D 33 C9 xor ecx, ecx
	// il2cpp : 000000018073710F E8 FC 3C 63 03                                      call    sub_183D6AE10
	// il2cpp : 0000000180737114 48 8B F8                                            mov     rdi, rax
	bool ResolveLibObject(DMA_Connection* Conn)
	{
		auto& proc = EFT::GetProcess();
		auto pid = proc.GetPID();

		uintptr_t base = proc.GetAssemblyAddress();
		size_t    size = proc.GetAssemblySize();

		if (!base || !size)
			return false;

		uintptr_t libObjectPtr = SignatureScanner::FindSignature(
			Conn,
			"48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 4C 24 ? C6 05 ? ? ? ? ? 45 32 FF",
			base,
			base + size,
			pid
		);

		uintptr_t readAddr = libObjectPtr + 3;

		int relative = proc.ReadMem<int>(Conn, readAddr);

		uintptr_t LibObject = libObjectPtr + 7 + relative;

		uintptr_t LibObjectRVA = LibObject - base;

		Offsets::ZLibObject = LibObjectRVA;

		std::println("[+] LibObject RVA Offset: 0x{:X}", Offsets::ZLibObject);

		return LibObject != 0;
	}

	bool ResolveOffsets(DMA_Connection* Conn)
	{
		if (!ResolveGOM(Conn))
			throw std::runtime_error("Failed to resolve GOM (Game Object Manager)");

		if (!ResolveCameras(Conn))
			throw std::runtime_error("Failed to resolve Cameras");

		if (!ResolveLibObject(Conn))
			throw std::runtime_error("Failed to resolve LibObject");

		std::println("All offsets are resolved");

		return true;
	}
}
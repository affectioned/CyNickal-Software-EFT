#include "pch.h"
#include "EFT.h"

bool EFT::Initialize(DMA_Connection* Conn)
{
	std::println("Initializing EFT module...");

	Proc.GetProcessInfo(Conn);


	return false;
}

const Process& EFT::GetProcess()
{
	return Proc;
}
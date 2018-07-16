#pragma once

#include "resource.h"

class BatchCommand {
	WORD wID;
	std::wstring batName;

	typedef BatchCommand _Myt;

public:
	BatchCommand(_In_ WORD wBatchResourceId);
	BatchCommand(_Inout_ _Myt &&other) = delete;
	_Myt& operator = (_Inout_ _Myt &&rhs) = delete;
	BatchCommand(_In_ const _Myt &other) = delete;
	_Myt& operator = (_In_ const _Myt &rhs) = delete;
	virtual ~BatchCommand() noexcept(false);

public:
	std::wifstream invokeAndGetStream();
	std::list<std::wstring> invokeAndGetLines();

private:
	std::wstring generateBatchName();
	void generateBatchFile();
};

#pragma once

#include "resource.h"

class BatchCommand {
	std::wstring batName;
	std::wstring batContent;

	typedef BatchCommand _Myt;

public:
	BatchCommand(_In_ WORD wBatchResourceId);
	BatchCommand(_In_opt_ const std::wstring& batchContent = L"");
	BatchCommand(_Inout_ _Myt &&other) = delete;
	_Myt& operator = (_Inout_ _Myt &&rhs) = delete;
	BatchCommand(_In_ const _Myt &other) = delete;
	_Myt& operator = (_In_ const _Myt &rhs) = delete;
	virtual ~BatchCommand() noexcept(false);

public:
	const std::wstring& GetName() const noexcept { return batName; }
	const std::wstring& GetContent() const noexcept { return batContent; }
	std::unique_ptr<std::wistream> invokeAndGetStream();
	std::list<std::wstring> invokeAndGetLines();

private:
	std::wstring generateBatchName();
	std::wstring loadBatchContent(_In_ WORD wBatchResourceId);
};

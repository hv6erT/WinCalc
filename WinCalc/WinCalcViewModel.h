#pragma once
#include "WinCalcViewModel.g.h"
#include "AdditionalButtonsView.h"

namespace winrt::WinCalc::implementation
{
    struct WinCalcViewModel : WinCalcViewModelT<WinCalcViewModel>
    {
        WinCalcViewModel();

        winrt::WinCalc::AdditionalButtonsView AdditionalButtonsView();

    private:
        WinCalc::AdditionalButtonsView _additionalButtonsView{ nullptr };
    };
}
namespace winrt::WinCalc::factory_implementation
{
    struct WinCalcViewModel : WinCalcViewModelT<WinCalcViewModel, implementation::WinCalcViewModel>
    {
    };
}

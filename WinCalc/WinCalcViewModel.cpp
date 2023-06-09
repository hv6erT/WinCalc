#include "pch.h"
#include "WinCalcViewModel.h"
#include "WinCalcViewModel.g.cpp"

namespace winrt::WinCalc::implementation{
    WinCalcViewModel::WinCalcViewModel() {
        _additionalButtonsView = winrt::make<WinCalc::implementation::AdditionalButtonsView>(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
    }
    winrt::WinCalc::AdditionalButtonsView WinCalcViewModel::AdditionalButtonsView(){
        return _additionalButtonsView;
    }
}

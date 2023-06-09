#pragma once
#include "AdditionalButtonsView.g.h"

namespace winrt::WinCalc::implementation
{
    struct AdditionalButtonsView : AdditionalButtonsViewT<AdditionalButtonsView>
    {
        AdditionalButtonsView() = delete;
        AdditionalButtonsView(winrt::Microsoft::UI::Xaml::Visibility const& state);

        winrt::Microsoft::UI::Xaml::Visibility State();
        void State(winrt::Microsoft::UI::Xaml::Visibility const& value);
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        private:
            winrt::Microsoft::UI::Xaml::Visibility _state{ winrt::Microsoft::UI::Xaml::Visibility::Collapsed };
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
    };
}
namespace winrt::WinCalc::factory_implementation
{
    struct AdditionalButtonsView : AdditionalButtonsViewT<AdditionalButtonsView, implementation::AdditionalButtonsView>
    {
    };
}

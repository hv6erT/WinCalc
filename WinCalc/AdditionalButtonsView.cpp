#include "pch.h"
#include "AdditionalButtonsView.h"
#include "AdditionalButtonsView.g.cpp"

namespace winrt::WinCalc::implementation{

    AdditionalButtonsView::AdditionalButtonsView(winrt::Microsoft::UI::Xaml::Visibility const& state) : _state{state} {}
    winrt::Microsoft::UI::Xaml::Visibility AdditionalButtonsView::State(){
        return _state;
    }
    void AdditionalButtonsView::State(winrt::Microsoft::UI::Xaml::Visibility const& value)
    {
        if (_state != value)
        {
            _state = value;
            _propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"State" });
        }
    }
    winrt::event_token AdditionalButtonsView::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return _propertyChanged.add(handler);
    }
    void AdditionalButtonsView::PropertyChanged(winrt::event_token const& token) noexcept
    {
        _propertyChanged.remove(token);
    }
}

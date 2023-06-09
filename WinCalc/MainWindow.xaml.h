#pragma once

#include "MainWindow.g.h"
#include "WinCalcViewModel.h"

#include "Calculator/calculator.hpp"


enum class IOMode {
    input = 0,
    output = 1
};

namespace winrt::WinCalc::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        //Binding class
        WinCalc::WinCalcViewModel MainViewModel();
    private:
        WinCalc::WinCalcViewModel _mainViewModel{ nullptr };
    private:
        //Backdrop setup functions
        winrt::Windows::System::DispatcherQueueController createSystemDispatcherQueueController();
        void setupSystemBackdropConfiguration();
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme convertToSystemBackdropTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& theme);

        //Windowing functions
        bool setMicaBackdropOnWindow();

        //Windowing
        struct Windowing {
            static winrt::Microsoft::UI::Windowing::AppWindow appWindow;
            static winrt::Microsoft::UI::Windowing::OverlappedPresenter appWindowPresenter;
            static winrt::Microsoft::UI::Xaml::FrameworkElement appWindowRoot;
            static constexpr unsigned int defaultWidth{ 260 };
            static constexpr unsigned int defaultHeight{ 420 };
            static constexpr float defaultSizeRatio{ static_cast<float>(defaultWidth) / defaultHeight };

            //Windowing::Backdrop
            struct Backdrop {
                //Mica effect controllers
                static winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController backdropController;
                static winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration backdropConfig;

                //Revokers and controllers
                static winrt::Microsoft::UI::Xaml::Window::Activated_revoker _activatedRevoker;
                static winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker _themeChangedRevoker;
                static winrt::Windows::System::DispatcherQueueController _dispatcherQueueController;
            };
        };
    
        //Additional
        bool isShiftPressed{ false };
        bool isSettingsEditingEnabled = false;

        //Calculations
        Calculator calc;
        Calculator::resolveOptions calcResolveOptions;

        //Input Output handle
        IOMode calcIOMode = IOMode::output;
        void updateCalcTextBlocks();
        void updateCalcTextBlocks(IOMode newIOMode);

        //History
        void updateCalcHistoryStackPanel();
        void clearCalcHistoryStackPanel();

        //Shift button 
        void toggleCalcShift();
        void unshiftCalc();

        //Settings
        Windows::Storage::ApplicationDataContainer settings = Windows::Storage::ApplicationData::Current().LocalSettings();
        void applySettings();
        void applySettingsToXaml();
        Windows::Foundation::IAsyncAction openLicenseFile();

    public:
        //Event handling
        void handleGlobalKeyUp(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void handleClearButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleBracketButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handlePercentButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleBackspaceButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleInverseButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleExp2ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleRootButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleDivideButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleMultiplicationButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleSubtractionButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleAdditionButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle9ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle8ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle7ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle6ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle5ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle4ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle3ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle2ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle1ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handle0ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleCommaButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleResultButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        
        //History panel
        void handleHistoryButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleHistoryPanelCloseButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleHistoryPanelClearButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleHistoryRestoreButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        
        //Settings panel
        void handleSettingsPanelCloseButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void handleSettingsButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        
        //Shift button
        void handleShiftToggleButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        
        //Settings
        void handleAppThemeSettingsComboBoxSelectionChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void handleDecimalRoundingSettingsSliderValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void handleSignificantRoundingSettingsSliderValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void handleLicenseButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
    };
}

namespace winrt::WinCalc::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}

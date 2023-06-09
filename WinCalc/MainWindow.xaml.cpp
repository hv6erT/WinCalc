#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WinCalc::implementation {
	//Constructor
	MainWindow::MainWindow() {
		//Components init
		this->InitializeComponent();
		_mainViewModel = winrt::make<WinCalc::implementation::WinCalcViewModel>();

		const auto windowNative{ this->try_as<IWindowNative>() };
		winrt::check_bool(windowNative);
		HWND hWnd{ 0 };
		windowNative->get_WindowHandle(&hWnd);
		const auto windowId = winrt::Microsoft::UI::GetWindowIdFromWindow(hWnd);
		Windowing::appWindow = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);
		Windowing::appWindowPresenter = Windowing::appWindow.Presenter().as<winrt::Microsoft::UI::Windowing::OverlappedPresenter>();
		Windowing::appWindowRoot = this->Content().try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>();

		//Settings init
		this->applySettings();

		//Window config
		Windowing::appWindow.Title(L"WinCalc");

		unsigned int dpi = GetDpiForWindow(hWnd);
		float scalingFactor = static_cast<float>(dpi) / 96;
		int width = static_cast<int>(Windowing::defaultWidth * scalingFactor);
		int height = static_cast<int>(Windowing::defaultHeight * scalingFactor);
		Windowing::appWindow.Resize({ width, height });

		const auto titleBar = Windowing::appWindow.TitleBar();
		titleBar.ButtonInactiveBackgroundColor(Microsoft::UI::Colors::Colors(nullptr).Transparent());
		titleBar.ButtonBackgroundColor(Microsoft::UI::Colors::Colors(nullptr).Transparent());
		titleBar.ExtendsContentIntoTitleBar(true);

		Windowing::appWindowPresenter.IsMaximizable(false);
		Windowing::appWindowPresenter.IsResizable(false);

		this->setMicaBackdropOnWindow();

		this->updateCalcTextBlocks(IOMode::input);
		this->applySettingsToXaml();
		this->isSettingsEditingEnabled = true;
	}
	//Windowing
	winrt::Microsoft::UI::Windowing::AppWindow MainWindow::Windowing::appWindow{ nullptr };
	winrt::Microsoft::UI::Windowing::OverlappedPresenter MainWindow::Windowing::appWindowPresenter{ nullptr };
	winrt::Microsoft::UI::Xaml::FrameworkElement MainWindow::Windowing::appWindowRoot{ nullptr };

	//Windowing::Backdrop
	winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController MainWindow::Windowing::Backdrop::backdropController{ nullptr };
	winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration MainWindow::Windowing::Backdrop::backdropConfig{ nullptr };

	winrt::Microsoft::UI::Xaml::Window::Activated_revoker MainWindow::Windowing::Backdrop::_activatedRevoker;
	winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker MainWindow::Windowing::Backdrop::_themeChangedRevoker;

	winrt::Windows::System::DispatcherQueueController MainWindow::Windowing::Backdrop::_dispatcherQueueController{ nullptr };

	//::
	WinCalc::WinCalcViewModel MainWindow::MainViewModel() {
		return _mainViewModel;
	}
	bool MainWindow::setMicaBackdropOnWindow() {
		//Mica Effect
		//copied from: https://github.com/MicrosoftDocs/windows-dev-docs/blob/docs/hub/apps/windows-app-sdk/system-backdrop-controller.md

		if (winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController::IsSupported())
		{
			if (nullptr == winrt::Windows::System::DispatcherQueue::GetForCurrentThread() && nullptr == Windowing::Backdrop::_dispatcherQueueController) {
				Windowing::Backdrop::_dispatcherQueueController = createSystemDispatcherQueueController();
			}

			setupSystemBackdropConfiguration();

			// Setup Mica on the current Window.
			Windowing::Backdrop::backdropController = winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController();
			Windowing::Backdrop::backdropController.Kind(winrt::Microsoft::UI::Composition::SystemBackdrops::MicaKind::Base);
			Windowing::Backdrop::backdropController.SetSystemBackdropConfiguration(Windowing::Backdrop::backdropConfig);
			Windowing::Backdrop::backdropController.AddSystemBackdropTarget(this->try_as<winrt::Microsoft::UI::Composition::ICompositionSupportsSystemBackdrop>());

			return true;
		}

		return false;
	}
	winrt::Windows::System::DispatcherQueueController MainWindow::createSystemDispatcherQueueController() {
		DispatcherQueueOptions options
		{
			sizeof(DispatcherQueueOptions),
			DQTYPE_THREAD_CURRENT,
			DQTAT_COM_NONE
		};

		::ABI::Windows::System::IDispatcherQueueController* ptr{ nullptr };
		winrt::check_hresult(CreateDispatcherQueueController(options, &ptr));
		return { ptr, take_ownership_from_abi };
	}
	void MainWindow::setupSystemBackdropConfiguration() {
		Windowing::Backdrop::backdropConfig = winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration();

		//Activation state.
		Windowing::Backdrop::_activatedRevoker = this->Activated(winrt::auto_revoke,
			[&](auto&&, Microsoft::UI::Xaml::WindowActivatedEventArgs const& args)
			{
				Windowing::Backdrop::backdropConfig.IsInputActive(
					winrt::Microsoft::UI::Xaml::WindowActivationState::Deactivated != args.WindowActivationState());
			});

		Windowing::Backdrop::backdropConfig.IsInputActive(true);

		//Application theme.
		if (nullptr != Windowing::appWindowRoot)
		{
			Windowing::Backdrop::_themeChangedRevoker = Windowing::appWindowRoot.ActualThemeChanged(winrt::auto_revoke,
				[&](auto&&, auto&&)
				{
					Windowing::Backdrop::backdropConfig.Theme(
						convertToSystemBackdropTheme(Windowing::appWindowRoot.ActualTheme()));
				});

			Windowing::Backdrop::backdropConfig.Theme(convertToSystemBackdropTheme(Windowing::appWindowRoot.ActualTheme()));
		}
	}
	winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme MainWindow::convertToSystemBackdropTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& theme) {
		switch (theme)
		{
		case winrt::Microsoft::UI::Xaml::ElementTheme::Dark:
			return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Dark;
		case winrt::Microsoft::UI::Xaml::ElementTheme::Light:
			return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Light;
		default:
			return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Default;
		}
	}
	//Calculator view update
	void MainWindow::updateCalcTextBlocks() {
		if (this->calcIOMode == IOMode::input)
			CalcIOTextBlock().Text(this->calc.input);
		else
			CalcIOTextBlock().Text(this->calc.output);

		CalcONTextBlock().Text(this->calc.notation);
	}
	void MainWindow::updateCalcTextBlocks(IOMode newIOMode) {
		this->calcIOMode = newIOMode;

		MainWindow::updateCalcTextBlocks();
	}
	//Calculator history
	void MainWindow::updateCalcHistoryStackPanel() {
		auto node = BottomHistoryPanelContentStackPanel();

		for (size_t i = node.Children().Size(); i < this->calc.history.size(); i++) {
			winrt::Microsoft::UI::Xaml::Controls::Button button;

			winrt::Microsoft::UI::Xaml::Controls::TextBlock textBlock;
			textBlock.Text(this->calc.history[i]);
			textBlock.TextTrimming(winrt::Microsoft::UI::Xaml::TextTrimming::CharacterEllipsis);
			textBlock.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::NoWrap);

			button.Content(textBlock);

			button.Margin(winrt::Microsoft::UI::Xaml::Thickness(0, 2.5, 0, 2.5));
			button.MaxWidth(280);
			button.Click({ this, &MainWindow::handleHistoryRestoreButtonClick });

			node.Children().Append(button);
		}
	}
	void MainWindow::clearCalcHistoryStackPanel() {
		this->calc.history.clear();
		BottomHistoryPanelContentStackPanel().Children().Clear();
	}
	//Calculator shift
	void MainWindow::toggleCalcShift() {
		if (MainViewModel().AdditionalButtonsView().State() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
			MainViewModel().AdditionalButtonsView().State(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
			shiftToggleButton().IsChecked(false);
		}
		else if (MainViewModel().AdditionalButtonsView().State() == winrt::Microsoft::UI::Xaml::Visibility::Collapsed) {
			MainViewModel().AdditionalButtonsView().State(winrt::Microsoft::UI::Xaml::Visibility::Visible);
			shiftToggleButton().IsChecked(true);
		}

		this->isShiftPressed = !isShiftPressed;
	}
	void MainWindow::unshiftCalc() {
		if (MainViewModel().AdditionalButtonsView().State() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
			MainViewModel().AdditionalButtonsView().State(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
			shiftToggleButton().IsChecked(false);
		}

		this->isShiftPressed = false;
	}
	//Setings
	void MainWindow::applySettings() {
		//Theme
		winrt::Microsoft::UI::Xaml::ElementTheme currTheme = WindowGrid().RequestedTheme();
		auto reqTheme = this->settings.Values().Lookup(L"appTheme");

		if (reqTheme == nullptr) {
			this->settings.Values().Insert(L"appTheme", winrt::box_value(L"System default"));
			reqTheme = this->settings.Values().Lookup(L"appTheme");
		}

		hstring reqThemeValue = reqTheme.as<hstring>();
		if (reqThemeValue == L"System default") {
			if (currTheme != winrt::Microsoft::UI::Xaml::ElementTheme::Default)
				WindowGrid().RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Default);
		}
		else if (reqThemeValue == L"Light") {
			if (currTheme != winrt::Microsoft::UI::Xaml::ElementTheme::Light)
				WindowGrid().RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Light);
		}
		else if (reqThemeValue == L"Dark") {
			if (currTheme != winrt::Microsoft::UI::Xaml::ElementTheme::Dark)
				WindowGrid().RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Dark);
		}

		//Calculator settings
		auto decimalPlacesPrec = this->settings.Values().Lookup(L"decimalPlacesPrec");

		if (decimalPlacesPrec == nullptr) {
			this->settings.Values().Insert(L"decimalPlacesPrec", winrt::box_value(L"4"));
			decimalPlacesPrec = this->settings.Values().Lookup(L"decimalPlacesPrec");
		}

		try {
			hstring decimalPlacesPrecValue = decimalPlacesPrec.as<hstring>();
			this->calcResolveOptions.decimalPlacesPrec = std::stoll(to_string(decimalPlacesPrecValue));
		}catch(...){
			this->settings.Values().Clear();
			this->calcResolveOptions = Calculator::resolveOptions();
		}

		auto significantDigitsPrec = this->settings.Values().Lookup(L"significantDigitsPrec");

		if (significantDigitsPrec == nullptr) {
			this->settings.Values().Insert(L"significantDigitsPrec", winrt::box_value(L"8"));
			significantDigitsPrec = this->settings.Values().Lookup(L"significantDigitsPrec");
		}

		try {
			hstring significantDigitsPrecValue = significantDigitsPrec.as<hstring>();
			this->calcResolveOptions.significantDigitsPrec = std::stoll(to_string(significantDigitsPrecValue));

			this->calcResolveOptions.significantDigitsNotationPrec = this->calcResolveOptions.significantDigitsPrec;
		}catch(...){
			this->settings.Values().Clear();
			this->calcResolveOptions = Calculator::resolveOptions();
		}
	}
	void MainWindow::applySettingsToXaml() {
		//Theme
		auto appTheme = this->settings.Values().Lookup(L"appTheme");
		if(appTheme != nullptr)
			appThemeSettingsComboBox().SelectedItem(appTheme);

		//Calculator settings
		auto decimalPlacesPrec = this->settings.Values().Lookup(L"decimalPlacesPrec");

		if(decimalPlacesPrec != nullptr)
			decimalRoundingSettingsSlider().Value(std::stod(to_string(decimalPlacesPrec.as<hstring>())));

		auto significantDigitsPrec = this->settings.Values().Lookup(L"significantDigitsPrec");

		if (significantDigitsPrec != nullptr)
			significantRoundingSettingsSlider().Value(std::stod(to_string(significantDigitsPrec.as<hstring>())));
		
		auto packageInfo = Windows::ApplicationModel::Package::Current().Id();

		std::string appVersion = std::format(" {0}.{1}.{2}.{3} ", 
			packageInfo.Version().Major,
			packageInfo.Version().Minor,
			packageInfo.Version().Build,
			packageInfo.Version().Revision);

		appVersionTextBlock().Text(to_hstring(appVersion));
	}
	Windows::Foundation::IAsyncAction MainWindow::openLicenseFile() {
		Windows::Storage::StorageFolder installFolder{ Windows::ApplicationModel::Package::Current().InstalledLocation() };
		Windows::Storage::StorageFile file{ co_await installFolder.GetFileAsync(L"Metadata\\license.htm") };
		
		if (file){
			Windows::System::LauncherOptions launcherOptions;
			launcherOptions.DisplayApplicationPicker(true);

			bool success{ co_await Windows::System::Launcher::LaunchFileAsync(file, launcherOptions) };
		}
		else{
			// Couldn't find file.
		}
	}
}

//Event handling
void winrt::WinCalc::implementation::MainWindow::handleGlobalKeyUp(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e){
	if (e.Key() == winrt::Windows::System::VirtualKey::Shift)
		this->toggleCalcShift();
}

void winrt::WinCalc::implementation::MainWindow::handleClearButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		this->calc.clear();
	}
	else {
		this->calc.pushInputToNotation();

		this->calc.addFunctionToNotation(L"sin");
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleBracketButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		this->calc.addBracketToNotation();
	}
	else {
		this->calc.addFunctionToNotation(L"cos");
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handlePercentButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		if (this->calc.notation.length() > 0) {
			wchar_t lastSymbol = this->calc.notation[(this->calc.notation.length() - 1)];

			if (this->calc.isDigit(lastSymbol) || this->calc.isConstant(lastSymbol) || lastSymbol == L')' || lastSymbol == L'|') {
				this->calc.tryPutBracketsIfMissing();

				this->calc.notation = L"(" + this->calc.notation + L")";
				this->calc.addToNotation(L'×');
				this->calc.notation += L"100";
				this->calc.resolve(this->calcResolveOptions);

				this->updateCalcTextBlocks(IOMode::output);
			}
		}
	}
	else {
		this->calc.addFunctionToNotation(L"tg");
		this->unshiftCalc();
		this->updateCalcTextBlocks(IOMode::input);
		this->calc.pushInputToNotation();
	}
}

void winrt::WinCalc::implementation::MainWindow::handleBackspaceButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		if (this->calcIOMode == IOMode::output)
			this->updateCalcTextBlocks(IOMode::input);
		else if (this->calc.input == L"0" || this->calc.input.length() == 0) {
			calc.popLastNumberFromNotationToInput();
		}
		else {
			this->calc.input = this->calc.input.substr(0, (this->calc.input.length() - 1));
		}

		this->updateCalcTextBlocks(IOMode::input);
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addFunctionToNotation(L"ctg");
		this->unshiftCalc();
		this->updateCalcTextBlocks(IOMode::input);
	}
}

void winrt::WinCalc::implementation::MainWindow::handleInverseButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		if (this->calc.notation.length() > 0 && this->calc.notation != L"0") {
			wchar_t lastNotationSymbol = this->calc.notation[this->calc.notation.length() - 1];
			
			if (Calculator::isArithmeticSymbol(lastNotationSymbol)) {
				return;
			}
			else if (lastNotationSymbol == L'(') {
				this->calc.notation += L"1÷(";
			}
			else if (lastNotationSymbol == L'|' && std::count(this->calc.notation.begin(), this->calc.notation.end(), L'|') % 2 != 0) {
				this->calc.notation += L"1÷(";
			}
			else {
				if (this->calc.addToNotation(L'×')) {
					this->calc.notation += L"1÷(";
				}
			}
		}
		else
			this->calc.notation = L"1÷(";
	}
	else {
		this->calc.addPipeToNotation();
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleExp2ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		if (this->calc.addToNotation(L'^'))
			this->calc.notation += L"2";
	}
	else {
		this->calc.addToNotation(L'^');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleRootButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		this->calc.addToNotation(L'√');
	}
	else {
		this->calc.addToNotation(L'∛');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleDivideButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		this->calc.addToNotation(L'÷');
	}
	else {
		this->calc.addToNotation(L'%');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleMultiplicationButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		this->calc.addToNotation(L'×');
	}
	else {
		this->calc.addFunctionToNotation(L"log");
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleSubtractionButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	
	if (this->isShiftPressed == false) {
		if ((this->calc.notation == L"0" || this->calc.notation.length() == 0) && (this->calc.input == L"0" || this->calc.input.length() == 0)) {
			this->calc.input = L"-";
		}
		else if (this->calc.notation.length() > 0) {
			wchar_t lastNotationSymbol = this->calc.notation[this->calc.notation.length() - 1];
			if (this->calc.input.length() == 0 && (this->calc.isArithmeticSymbol(lastNotationSymbol) || lastNotationSymbol == L'(' || (lastNotationSymbol == L'|' && std::count(this->calc.notation.begin(), this->calc.notation.end(), L'|') % 2 == 1))) {
				this->calc.input = L"-";
			}
			else {
				this->calc.pushInputToNotation();
				this->calc.addToNotation(L'-');
			}
		}
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addFunctionToNotation(L"ln");
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleAdditionButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();

	if (this->isShiftPressed == false) {
		this->calc.addToNotation(L'+');
	}
	else {
		this->calc.addToNotation(L'!');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle9ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'9');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle8ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'8');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle7ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'7');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle6ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'6');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle5ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'5');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle4ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.addToInput(L'4');
	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle3ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		this->calc.addToInput(L'3');
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addToInput(L'φ');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle2ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		this->calc.addToInput(L'2');
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addToInput(L'γ');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle1ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		this->calc.addToInput(L'1');
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addToInput(L'e');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handle0ButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false){
		this->calc.addToInput(L'0');
	}
	else {
		this->calc.pushInputToNotation();
		this->calc.addToInput(L'π');
		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleCommaButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (this->isShiftPressed == false) {
		this->calc.addToInput(L'.');
	}
	else {
		this->calc.pushInputToNotation();

		if (this->calc.addToNotation(L'×')) {
			this->calc.notation += L"10^";
		}

		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::input);
}

void winrt::WinCalc::implementation::MainWindow::handleResultButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->calc.pushInputToNotation();
	this->calc.tryPutBracketsIfMissing();

	if(this->isShiftPressed == false)
		this->calc.resolve(this->calcResolveOptions);
	else {
		Calculator::resolveOptions calcRoundingResolveOptions;
		calcRoundingResolveOptions.roundingOrder = { Calculator::roundingType::significantDigits, Calculator::roundingType::significantDigitsNotation };
		calcRoundingResolveOptions.roundingOrderLimits = { 40 };

		calcRoundingResolveOptions.significantDigitsPrec = this->calcResolveOptions.significantDigitsPrec;
		calcRoundingResolveOptions.significantDigitsNotationPrec = this->calcResolveOptions.significantDigitsNotationPrec;

		this->calc.resolve(calcRoundingResolveOptions);

		this->unshiftCalc();
	}

	this->updateCalcTextBlocks(IOMode::output);
	this->updateCalcHistoryStackPanel();
}

void winrt::WinCalc::implementation::MainWindow::handleHistoryButtonClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	if (BottomHistoryPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Collapsed) {
		BottomHistoryPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Visible);

		handleSettingsPanelCloseButton(sender, e);
		auto fillColorBrush = Application::Current().Resources().Lookup(winrt::box_value(L"FilledAppBackgroundBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
		WindowGrid().Background(fillColorBrush);

		BottomHistoryPanelStoryboard().Begin();
	}
	else if (BottomHistoryPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
		BottomHistoryPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
		WindowGrid().Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Transparent()));
	}
}

void winrt::WinCalc::implementation::MainWindow::handleHistoryPanelCloseButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (BottomHistoryPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
		BottomHistoryPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
		WindowGrid().Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Transparent()));
	}
}

void winrt::WinCalc::implementation::MainWindow::handleHistoryPanelClearButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	clearCalcHistoryStackPanel();
}

void winrt::WinCalc::implementation::MainWindow::handleHistoryRestoreButtonClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	hstring textHstr = sender.try_as<winrt::Microsoft::UI::Xaml::Controls::Button>().Content().try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>().Text();
	this->calc.notation = static_cast<std::wstring>(textHstr);

	this->calc.resolve(this->calcResolveOptions);
	this->updateCalcTextBlocks(IOMode::output);
	MainWindow::handleHistoryPanelCloseButton(sender, e);
}

void winrt::WinCalc::implementation::MainWindow::handleSettingsButtonClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	if (BottomSettingsPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Collapsed) {
		BottomSettingsPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Visible);

		MainWindow::handleHistoryPanelCloseButton(sender, e);
		auto fillColorBrush = Application::Current().Resources().Lookup(winrt::box_value(L"FilledAppBackgroundBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
		WindowGrid().Background(fillColorBrush);

		BottomSettingsPanelStoryboard().Begin();
	}
	else if (BottomSettingsPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
		BottomSettingsPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
		WindowGrid().Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Transparent()));
	}
}

void winrt::WinCalc::implementation::MainWindow::handleSettingsPanelCloseButton(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	if (BottomSettingsPanelGrid().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible) {
		BottomSettingsPanelGrid().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
		WindowGrid().Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Transparent()));
	}
}

void winrt::WinCalc::implementation::MainWindow::handleShiftToggleButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
	this->toggleCalcShift();
}

void winrt::WinCalc::implementation::MainWindow::handleAppThemeSettingsComboBoxSelectionChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e){
	if (this->isSettingsEditingEnabled == false)
		return;

	this->settings.Values().Insert(L"appTheme", winrt::box_value(appThemeSettingsComboBox().SelectedValue()));
	this->applySettings();
}

void winrt::WinCalc::implementation::MainWindow::handleDecimalRoundingSettingsSliderValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e){
	if (this->isSettingsEditingEnabled == false)
		return;
	
	this->settings.Values().Insert(L"decimalPlacesPrec", winrt::box_value(to_hstring(decimalRoundingSettingsSlider().Value())));
	this->applySettings();
}

void winrt::WinCalc::implementation::MainWindow::handleSignificantRoundingSettingsSliderValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e){
	if (this->isSettingsEditingEnabled == false)
		return;
	
	this->settings.Values().Insert(L"significantDigitsPrec", winrt::box_value(to_hstring(significantRoundingSettingsSlider().Value())));
	this->applySettings();
}

void winrt::WinCalc::implementation::MainWindow::handleLicenseButtonClick(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
	this->openLicenseFile();
}

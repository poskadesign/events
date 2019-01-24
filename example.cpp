struct WidgetEventArgs {
    explicit WidgetEventArgs(std::string _reversedString) : reversedString(_reversedString) { }
    std::string reversedString;
};

class Widget {
public:
    void reverseString(std::string s) {
        auto reversedS = std::string(s.rbegin(), s.rend());

        // fire stringReversed event
        // method 1:
        stringReversed.fire(WidgetEventArgs(reversedS));

        // method 2:
        stringReversed(WidgetEventArgs(reversedS));

    }
    Event<WidgetEventArgs> stringReversed;
};

class Consumer {
public:
    Consumer() {
        // event subscription to a member function
        _widget.stringReversed.bind(&Consumer::_onStringReversed1, this);
        _widget.stringReversed.bind(&Consumer::_onStringReversed2, this);

        // ignore subscription if handler function already subscribed:
        _widget.stringReversed.bind(&Consumer::_onStringReversed2, this, EventFlag::ONLY_UNIQUE);

        // unsubscribe event
        _widget.stringReversed.unbind(&Consumer::_onStringReversed2, this);

        // subscribe a lambda
        _widget.stringReversed += [](auto e) {
            std::cout << e.reversedString << " from lambda\n";
        };

        _widget.reverseString("Hello");

    }

private:
    Widget _widget;

    void _onStringReversed1(WidgetEventArgs e) {
        std::cout << e.reversedString << 1 << std::endl;
    }

    void _onStringReversed2(WidgetEventArgs e) {
        std::cout << e.reversedString << 2 << std::endl;
    }
};


int main() {
    Consumer c;
    return 0;
}

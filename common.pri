# Find Boost library.

# Try to use qmake variable's value.
_BOOST_ROOT = $$BOOST_ROOT
isEmpty(_BOOST_ROOT) {
    message(\"Boost Library\" qmake value not detected...)

    # Try to use the system environment value.
    _BOOST_ROOT = $$(BOOST_ROOT)
}

isEmpty(_BOOST_ROOT) {
    message(\"Boost Library\" environment variable not detected...)
    !build_pass:error(Please set the environment variable `BOOST_ROOT`. For example, BOOST_ROOT=c:\\boost_1_53_0)
} else {
    message(\"Boost Library\" detected in BOOST_ROOT = \"$$_BOOST_ROOT\")

    INCLUDEPATH += $$_BOOST_ROOT
    LIBS += -L$$_BOOST_ROOT/stage/lib/

    win32:CONFIG(release, debug|release): LIBS += -llibboost_system-vc100-mt-1_55
    else:win32:CONFIG(debug, debug|release): LIBS += -llibboost_system-vc100-mt-gd-1_55
    else:unix: LIBS += -lboost_system

}

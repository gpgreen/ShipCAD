# Find Eigen library.

# Try to use qmake variable's value.
_EIGEN_ROOT = $$EIGEN_ROOT
isEmpty(_EIGEN_ROOT) {
    # Try to use the system environment value.
    _EIGEN_ROOT = $$(EIGEN_ROOT)
}

isEmpty(_EIGEN_ROOT) {
    message(\"Eigen Library\" environment variable (EIGEN_ROOT) not detected...)
    !build_pass:error(Please set the environment variable `EIGEN_ROOT`. For example, EIGEN_ROOT=c:\\src\\eigen)
} else {
    INCLUDEPATH += $$_EIGEN_ROOT
}

# Find Boost library.

# Try to use qmake variable's value.
_BOOST_ROOT = $$BOOST_ROOT
isEmpty(_BOOST_ROOT) {
    # Try to use the system environment value.
    _BOOST_ROOT = $$(BOOST_ROOT)
}

isEmpty(_BOOST_ROOT) {
    message(\"Boost Library\" environment variable (BOOST_ROOT) not detected...)
    !build_pass:error(Please set the environment variable `BOOST_ROOT`. For example, BOOST_ROOT=c:\\src\\boost)
} else {
    INCLUDEPATH += $$_BOOST_ROOT
}

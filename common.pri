# Find Eigen library.

# Try to use qmake variable's value.
_EIGEN_ROOT = $$EIGEN_ROOT
isEmpty(_EIGEN_ROOT) {
    message(\"Eigen Library\" qmake value not detected...)

    # Try to use the system environment value.
    _EIGEN_ROOT = $$(EIGEN_ROOT)
}

isEmpty(_EIGEN_ROOT) {
    message(\"Eigen Library\" environment variable not detected...)
    !build_pass:error(Please set the environment variable `EIGEN_ROOT`. For example, EIGEN_ROOT=c:\\src\\eigen)
} else {
    message(\"Eigen Library\" detected in EIGEN_ROOT = \"$$_EIGEN_ROOT\")

    INCLUDEPATH += $$_EIGEN_ROOT
}

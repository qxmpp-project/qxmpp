<!--
SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>

SPDX-License-Identifier: CC0-1.0
-->

# Contributing

This is a guideline for contributing to QXmpp.

## Git Hooks Setup

QXmpp currently just provides a hook for code formatting before commiting.
You can enable it as follows:
```
cd qxmpp
./utils/setup-hooks.sh
```
This requires `git-clang-format` to be installed, though.

## Pull-requests (PRs)

You can submit suggestions for changes via GitHub pull-requests.
The maintainers of QXmpp will review your code. However, everybody is welcome to review the code and give feedback.
You can find the current maintainers and other people involved in the project in the `AUTHORS` file.

### Guidelines for Creating PRs

1. Do not use the *master* branch of your personal fork to create PRs. It leads to confusion and
   makes it harder to check out your branch, making changes to it and in the end also merging your
   PR into the *master* branch of the main repository.
1. For fixes, target the branch of the latest release (e.g. *1.3*) instead of the *master* branch.
1. Write short commit messages starting with an upper case letter and the imperative.
1. Split your commits logically.
1. Do not mix unrelated changes in the same PR.
1. Check the tasks in the pull-request template when they are done or no changes were needed.

## Testing

After building QXmpp with `-DBUILD_TESTS=ON`, you can run QXmpp's unit tests in the build directory with:
```
make test
```

### Internal Tests

There are some additional internal tests that only work with debug builds because they are using unexported symbols from the QXmpp library.
You can enable those using `-DBUILD_INTERNAL_TESTS=ON`.
They are also run using `make test`, along with the regular tests.

### Integration Tests

There are some tests that also provide integration testing with a real XMPP server.
The code of those test cases is always built, but the cases are skipped when running `make test` without further configuration.

You can enable them by exporting the following environment variables:

* `QXMPP_TESTS_INTEGRATION_ENABLED=1`
* `QXMPP_TESTS_JID=<jid>`
* `QXMPP_TESTS_PASSWORD=<password>`

Replace `<jid>` and `<password>` with the credentials of the account for connecting to the server.

## Documentation

The documentation is generated using Doxygen.
You can enable it using the CMake option `-DBUILD_DOCUMENTATION=ON` and build it manually using:
```
make doc
```

The generated files are located in `doc/html/`.
You can open `doc/html/index.html` for reading the documentation.

To correctly view the list of supported XEPs, a local webserver is needed, see the [DOAP rendering documentation][doap-rendering] for details.

## Copyright

QXmpp is [REUSE-compatible][reuse].
Thus, you need to add an appropriate copyright header to each new file.
If do significant changes to a file (e.g. no search and replace, typo fixes or just adding of pure
non-copyrightable data like XML namespaces), you may add yourself as a copyright holder in the
copyright header. This is recommended for large changes.

If the file does not allow such a header, add a `.license` file for the copyright information.

[doap-rendering]: /doc/doap-rendering/README.md
[reuse]: https://api.reuse.software/info/github.com/qxmpp-project/qxmpp

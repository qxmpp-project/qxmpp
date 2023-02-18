#!/bin/sh

# SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
#
# SPDX-License-Identifier: CC0-1.0

# This script sets up useful git hooks in the local repository.

REPO_ROOT=$(dirname "$(readlink -f "${0}")")/..

cp $REPO_ROOT/utils/pre-commit.sh .git/hooks/pre-commit

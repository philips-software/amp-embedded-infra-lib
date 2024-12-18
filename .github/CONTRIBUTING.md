# Contributing to amp-embedded-infra-lib

First off, thank you for considering contributing to amp-embedded-infra-lib!

Following these guidelines helps to communicate that you respect the time of the developers managing and developing this open source project. In return, they should reciprocate that respect in addressing your issue, assessing changes, and helping you finalize your pull requests.

## Contributing to development

amp-embedded-infra-lib is a young open source project and we love to receive contributions from our community — you! There are many ways to contribute, from writing examples, improving the documentation, submitting bug reports and feature requests or writing code which can be incorporated into amp-embedded-infra-lib itself.

## Ground Rules

* Be professional, respectful and considerate in your communication with others.
* As amp-embedded-infra-lib core-team we have spent considerate effort to make our components testable; please make sure that you have test coverage for your changes as well. Inspiration can be drawn from existing components.
* One of the unique selling points of amp-embedded-infra-lib is not using any heap; don't use the heap in your changes. This excludes using most components from the standard template library, for which we provide heap-less alternatives.
* Ensure cross-platform compatibility for your changes.
* Adhere to the [Coding Standard C++ Embedded Projects](https://github.com/philips-software/amp-embedded-infra-lib/blob/main/documents/modules/ROOT/pages/CodingStandard.adoc); when in doubt, look at the surrounding code that you are changing and stick to the local style.
* Create issues for any major changes and enhancements that you wish to make. Discuss things transparently and get core-team feedback.

### Test are not optional

We can't stress this enough. Any bugfix that doesn’t include a test proving the existence of the bug being fixed, may be suspect. Ditto for new features that can’t prove they actually work. We’ve found that test-first development really helps make features better architected and identifies potential edge cases earlier instead of later. Writing tests before the implementation is strongly encouraged.

## Your First Contribution

### Introduction

Submitting your first contribution (or Pull Request) can be scary, but we promise you it will be rewarding. Information on how to proceed can be found on numerous blog posts and websites. A small selection of beginner friendly tutorials:

**Working on your first Pull Request?** You can learn how from this *free* series [How to Contribute to an Open Source Project on GitHub](https://app.egghead.io/playlists/how-to-contribute-to-an-open-source-project-on-github)

**Contributing to open source for the first time can be scary and a little overwhelming.** Perhaps you’re a [Code Newbie](https://www.codenewbie.org/) or maybe you’ve been coding for a while but haven’t found a project you felt comfortable contributing to. [You can do it; here's how.](https://www.firsttimersonly.com/)

### Naming a Pull Request (PR)

The title of your Pull Request (PR) should follow the style of [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/). Not only does this present a standardized categorization of the kind of work done on a pull request, but it also instructs the release workflow to increment the correct level of the version according to the rules of [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The format of the title of the pull request is this:

 `<type>[(optional scope)][!]: <description>`

The `<type>` of the pull request is one of these, taken from [conventional commit types](https://github.com/commitizen/conventional-commit-types):

- `feat:` a new feature
- `fix:` a bug fix
- `docs:` documentation only changes
- `style:` changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)
- `refactor:` a code change that neither fixes a bug nor adds a feature
- `perf:` a code change that improves performance
- `test:` adding missing tests or correcting existing tests
- `build:` changes that affect the build system or external dependencies
- `ci:` changes to our CI configuration files and scripts
- `chore:` other changes that don't modify source or test files
- `revert:` reverts a previous commit

An exclamation mark `!` is added to the type if the change is not backwards compatible. This should only be added to `feat` or `fix`.

> [!NOTE]
> We do not enforce conventional commits for individual commit messages, only for the title of your pull request.

Examples:

- `feat: add required-tool to devcontainer`

   This pull request adds the "required-tool" to the devcontainer because everybody want to use it.

- `fix!: escape fe ff in binary ports`

   This pull request fixes binary ports, and indicates that this is a backwards-incompatible change.

> [!TIP]
> If your work consists of a single commit, creating a pull request will default to the name of that commit. If you use conventional commit style for that single commit, your pull request already has the correct name.

### Full example

Make sure you have all development dependencies installed as described in the [README](https://github.com/philips-software/amp-embedded-infra-lib?tab=readme-ov-file).

#### Preparing your Fork

1. Click ‘Fork’ on Github, creating e.g. yourname/amp-embedded-infra-lib.
2. Clone your project: ```git clone https://github.com/yourname/amp-embedded-infra-lib```.
3. ```cd amp-embedded-infra-lib```.
4. Create the build directory ```cmake -E make_directory Build```.
5. Configure the project with CMake ```cmake ..```.
6. Build the project ```cd .. && cmake --build Build```.
7. Run the tests; when using Visual Studio ```cmake --build Build --target RUN_TESTS```, when using Ninja ```cmake --build Build --target test```.
8. Create a branch: git checkout -b foo-the-bars 1.3.

#### Making your Changes
1. Write tests expecting the correct/fixed functionality; make sure they fail.
2. Implement functionality to make the test pass.
3. Run tests again, making sure they pass.
4. Add changelog entry briefly describing your change.
5. Commit your changes: git commit -m "Foo the bars"

#### Creating Pull Requests
1. Push your commit to get it back up to your fork: git push origin HEAD
2. Visit Github, click handy “Pull request” button that it will make upon noticing your new branch.
3. In the description field, write down issue number (if submitting code fixing an existing issue) or describe the issue + your fix (if submitting a wholly new bugfix).
4. Hit ‘submit’. And please be patient - the maintainers will get to you when they can.

## How to report a bug

If you discover a vulnerability in our software, please refer to the [security policy](https://github.com/philips-software/amp-embedded-infra-lib?tab=security-ov-file) and report it appropriately.
Do not submit an issue, unless asked to.

In order to determine whether you are dealing with a security issue, ask yourself these two questions:
* Can I access something that's not mine, or something I shouldn't have access to?
* Can I disable something for other people?

If the answer to either of those two questions are "yes", then you're probably dealing with a security issue. Note that even if you answer "no" to both questions, you may still be dealing with a security issue, so if you're unsure, just email us.

When filing an issue, make sure to answer these five questions:

1. What compiler and compiler version are you using?
2. What operating system and processor architecture are you using?
3. What did you do?
4. What did you expect to see?
5. What did you see instead?

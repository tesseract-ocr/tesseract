# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 5.5.x   | :white_check_mark: |
| < 5.5   | :x:                |

## Reporting a Vulnerability

Please prefer GitHub's private vulnerability reporting, which keeps the
report private until a fix is available, credits you in the published
advisory, and can receive a CVE. Open the repository's "Security and
quality" tab and click "Report a vulnerability".

If you do not have a GitHub account, you may instead e-mail one of the
current maintainers. See the
[latest releases](https://github.com/tesseract-ocr/tesseract/releases)
and the Git history for their names and e-mail addresses.

In your report, please include your full name or the handle you would
like to receive as credit in the fix commit and the advisory. Omit it
if you prefer to remain anonymous; we will acknowledge the report
privately in that case. Also tell us whether and how AI was involved in
finding or writing up the report (for example: found by a fuzzer,
analyzed or verified with an AI tool, or report text AI-generated), so
we know how much verification we should put into the report before
acting on it.

## Pacing reports

We handle security reports as they arrive, with limited maintainer
resources, and each report needs a reproduction, a fix, and a test
before it can be closed. To make sure every report gets the attention
it needs, please process your reports one at a time: wait for a
response or a fix for your existing reports before submitting new
ones. If you have found several issues, it is usually better to group
them into a single report than to send a series of separate reports;
if you have a large number of findings (for example from a fuzzing
campaign), contact a maintainer in a first report and coordinate how
to proceed. Note that GitHub also enforces its own rate limits on
private vulnerability reports, so bursts of reports may simply be
rejected.

The POSIX "diff" and "patch" tools work line-oriented, but sometimes it would be preferable to work with word- or even character-based differences in the same way.

This project will therefore create a pre-/postprocessor, losslessly transforming input files into a format which contains only one word, one character or a one-byte hex-dump per text line.

We will also exploit fact that POSIX "diff" provides the "-b" option and POSIX "patch" provides the "-l" option, which make all sequences of one or more whitespaces (except newlines) at the end of a line compare equally as single word-separators.

The "word" transformation format shall therefore keep the original whitespace between words at the end of the lines.

The "character" transformation format shall do the same, so that those lines will in fact contain 2 or more characters rather than just one.

A challenge for all transformation formats are newline characters, though: We also want to compare any sequence of newline characters as if it were just a single whitespace.

And finally the case that the last line of the original file did not end with a newline character needs also be supported.

I propose a prefixed line format for transformation output, so that the following sample text (where the '$' represent newline characters for better visualizing trailing whitspace; the '$' are not present in the real input or output)

-----
Hi, world!$
$
This is a test.$
$
There are 3 empty lines after this one:$
$
$
$
End.$
-----

will be transformed into the following output when operating in "word" splitting mode:

-----
:Hi, $
:world!$
n$
:This is a test.$
$
:There $
:are $
:3 $
:empty $
:lines $
:after $
:this $
:one:$
n  $
:End.$
-----

The output would be the same in character-split mode, only that the ":" lines would only contain single character "words".

Here, lines start either with ":" which represents a word or character, or with "n" which represent adjacent newline sequences.

The "n" can be followed by one or more spaces which specify a repetition count for the newlines: No space means the line just represents a single newline sequence, one space means it represents two newlines, two spaces mean 3 newlines, and so on.

This means, however, that whitespace after a newline sequence needs to be represented differently.

Whitespace after a newline is represented by a word consisting only of whitespace.

The hexdump transformation format will be simpler than the format above: All lines consist of a 2-digit upper-case hexadecimal number representing the next byte, an will be followed with a space and the ASCII representation of that byte (or an ASCII dot if the byte does not correspond to a printable ASCII character).

Actually there will be 2 dump formats: one with (-x) and one without (-b) the ASCII dump.

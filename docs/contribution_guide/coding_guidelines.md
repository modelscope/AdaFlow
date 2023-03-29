# Coding Guidelines
In general, Adaflow follows [The Coding Style of GStreamer](https://gstreamer.freedesktop.org/documentation/frequently-asked-questions/developing.html?gi-language=python#what-is-the-coding-style-for-gstreamer-code) 
for coding convention.

## C codes(.c sources)
All C codes of Adaflow are required to use K&R with 2-space indenting.
We only require .c files to be indented, headers may be indented differently 
for better readability. Please use spaces for indenting, not tabs, even in header files.

## C headers (.h)
You may indent differently from what gst-indent does. You may also break the 80-column rule with header files.

## C++ files (.cc)
Do not use .cpp extensions, use .cc extensions for C++ sources. Use .h for headers.

Please try to stick with the same indentation rules (2 spaces) and refer to .clang-format, which mandates the coding styles via CI.

## Python files(.py)
We follow the [PEP8 style guide](https://peps.python.org/pep-0008/) for Python. Docstrings follow [PEP257](https://peps.python.org/pep-0257/). 

You can use tool [Flake8](https://flake8.pycqa.org/en/latest/) for style guide enforcement.
Please make sure you have installed Flake8.
```bash
pip install flake8
```
Apply flake8 to .py files.
```bash
flake8 --max-line-length=128 --show-source --statistics <file-name>
```
The output is formatted as:
```bash
file path : line number : column number : error code : short description
```
then you can modify code according to the prompt.







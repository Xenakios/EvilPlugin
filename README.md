# EvilPlugin

JUCE-based audio plugin that does all kinds of things plugins are not supposed to be doing, 
such as :

- Crashing from accessing invalid heap memory or by a stack overflow
- Locking up by making threads sleep or waiting for mutexes
- Using too much CPU in the GUI thread, the audio thread or a worker thread started by the plugin
- Using global variables in an inappropriate way to cause problems when multiple instances of the plugin
  is used in a multithreading host
- Division by zero
- Leak memory

It could be useful for amusement purposes or
for testing things like hosts that can host plugins in external processes to see how they handle the 
out-of-process plugins doing crazy things.

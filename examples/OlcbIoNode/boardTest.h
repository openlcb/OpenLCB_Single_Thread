#if defined(ARDUINO_AVR_RS_IO)
  #pragma message("Compiling for the RailStars Io")
#else
  #error("This sketch is specific to the RailStars Io")
#endif // Io test

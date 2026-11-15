# Matter mapping

The UART parser and Matter layer are deliberately separate. The parser maintains total active
power and total energy separately from phase-specific voltage/current. Do not replicate a total
power/energy reading into each phase.

For the selected ESP-Matter release, implement:
- Electrical Power Measurement cluster 0x0090, including PolyphasePower if supported.
- Electrical Energy Measurement cluster 0x0091 with the intended imported/exported energy features.
- Proper Matter units/scales, nullable values, feature maps, reporting and endpoint/device type.
- Thread commissioning and network setup using that release's documented application pattern.

`main/matter_meter.cpp` is intentionally a stub until the exact ESP-Matter release is selected.

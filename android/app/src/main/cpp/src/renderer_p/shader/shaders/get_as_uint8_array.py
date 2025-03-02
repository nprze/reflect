with open("cube/cube_vert.spv", "rb") as f:
    spv_data = f.read()
    print("const uint8_t shaderSpv[] = {")
    print(", ".join(f"0x{byte:02X}" for byte in spv_data))
    print("};")
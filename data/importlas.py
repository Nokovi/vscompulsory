import laspy

# Angi filnavn
input_file = '33-1-516-330-54.laz'           # Din LAZ-fil
output_file = 'lasdata1.txt'   # Utdatafil i ASCII-format

# Les LAZ-filen
las = laspy.read(input_file, laz_backend=laspy.LazBackend.Laszip)

# Skriv til ASCII-fil

with open(output_file, 'w') as f:
    #f.write("X Y Z Intensity Classification\n")  # Valgfri overskrift
    for x, y, z in zip(las.x, las.y, las.z):
        f.write(f"{x} {y} {z} \n")

print(f"LAZ-filen '{input_file}' er eksportert til ASCII-format i filen '{output_file}'.")


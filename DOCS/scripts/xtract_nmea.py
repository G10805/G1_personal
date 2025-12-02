def search_and_extract_to_file():
    filename = input("Enter the file name: ")
    search_word = input("Enter the word to search for: ")
    start_char = input("Enter the character to start printing from: ")
    output_filename = "extracted_lines.txt"

    try:
        with open(filename, 'r') as file:
            lines = file.readlines()

        extracted_lines = []
        for line in lines:
            if search_word in line:
                start_index = line.find(start_char)
                if start_index != -1:
                    extracted_lines.append(line[start_index:].strip())

        with open(output_filename, 'w') as output_file:
            for extracted_line in extracted_lines:
                output_file.write(extracted_line + '\n')

        print(f"Extracted lines have been written to '{output_filename}'.")

    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Run the function
search_and_extract_to_file()

def remove_duplicate_lines(input_file, output_file):
    try:
        with open(input_file, 'r') as file:
            lines = file.readlines()

        # Use a set to track seen lines and preserve order with a list
        seen = set()
        unique_lines = []
        for line in lines:
            if line not in seen:
                seen.add(line)
                unique_lines.append(line)

        with open(output_file, 'w') as file:
            file.writelines(unique_lines)

        print(f"Duplicate lines removed. Unique lines written to '{output_file}'.")

    except FileNotFoundError:
        print(f"File '{input_file}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage
remove_duplicate_lines('extracted_lines.txt', 'unique_lines.txt')

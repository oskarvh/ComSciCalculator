import os
import re

def copy_html_file_from_template():
    """
    Copy the index_template.html to index.html if it doesn't exist.
    """
    template_path = os.path.join(os.path.dirname(__file__), "index_template.html")
    target_path = os.path.join(os.path.dirname(__file__), "index.html")
    if not os.path.exists(target_path):
        if not os.path.exists(template_path):
            raise FileNotFoundError(f"Template file {template_path} does not exist.")
        with open(template_path, "r") as f:
            content = f.read()
        with open(target_path, "w") as f:
            f.write(content)
        print(f"Copied {template_path} to {target_path}")
    else:
        print(f"{target_path} already exists, not overwriting.")
    return target_path

def populate_html_with_fonts(html_file, font_dir=os.path.join(os.path.dirname(__file__), "../../common/gui/fonts")):
    """
    Populate the html file with font preload and import statements.
    """
    html_file = os.path.abspath(html_file)
    font_dir = os.path.abspath(font_dir)
    if not os.path.exists(html_file):
        raise FileNotFoundError(f"HTML file {html_file} does not exist.")
    if not os.path.exists(font_dir):
        raise FileNotFoundError(f"Font directory {font_dir} does not exist.")
    ttf_files = [f for f in os.listdir(font_dir) if f.lower().endswith(".ttf")]
    if not ttf_files:
        raise FileNotFoundError(f"No TTF files found in font directory {font_dir}.")
    with open(html_file, "r") as f:
        content = f.read()
    preload_lines = "\n".join([f'    <link rel="preload" href="/common/gui/fonts/{f}" as="font" type="font/ttf" crossorigin>' for f in ttf_files])
    import_lines = "\n".join([f"        @font-face {{ font-family: '{os.path.splitext(f)[0]}'; src: url('/common/gui/fonts/{f}'); font-weight: 400; font-style: normal; }}" for f in ttf_files])
    content = re.sub(
        r"<!-- START PRELOAD FONTS -->.*<!-- END PRELOAD FONTS -->", 
        f"<!-- START PRELOAD FONTS -->\n{preload_lines}\n    <!-- END PRELOAD FONTS -->", 
        content, 
        flags=re.DOTALL
    )
    content = re.sub(
        r"/\* <START IMPORT FONTS> \*/.*?/\* <END IMPORT FONTS> \*/", 
        f"/* <START IMPORT FONTS> */\n{import_lines}\n        /* <END IMPORT FONTS> */",
        content, 
        flags=re.DOTALL
    )
    with open(html_file, "w") as f:
        f.write(content)
    print(f"Populated {html_file} with {len(ttf_files)} fonts from {font_dir}.")
    return html_file


if __name__ == "__main__":
    # Copy the template file:
    html_file = copy_html_file_from_template()
    # Populate the html file with fonts:
    populate_html_with_fonts(html_file)
    print("Setup complete.")

import os
import re
import json

def copy_html_file_from_template():
    """
    Copy the index_template.html to index.html if it doesn't exist.
    """
    template_path = os.path.join(os.path.dirname(__file__), "index_template.html")
    target_path = os.path.join(os.path.dirname(__file__), "index.html")
    
    if not os.path.exists(template_path):
        raise FileNotFoundError(f"Template file {template_path} does not exist.")
    with open(template_path, "r") as f:
        content = f.read()
    with open(target_path, "w") as f:
        f.write(content)
    print(f"Copied {template_path} to {target_path}")
    return target_path

def populate_html_with_fonts(html_file, font_dir=os.path.join(os.path.dirname(__file__), "../../common/gui/fonts")):
    """
    Populate the html file with font preload and import statements.
    """
    html_file = os.path.abspath(html_file)
    font_dir = os.path.abspath(font_dir)
    fonts_lut = os.path.join(os.path.dirname(__file__), "../../common/gui/fonts/font_library/fonts.json")
    if not os.path.exists(html_file):
        raise FileNotFoundError(f"HTML file {html_file} does not exist.")
    if not os.path.exists(fonts_lut):
        raise FileNotFoundError(f"Font directory {fonts_lut} does not exist.")
    fonts_metadata = json.loads(open(fonts_lut, "r").read())
    with open(html_file, "r") as f:
        content = f.read()
    preload_lines = ""
    import_lines = ""
    for font in fonts_metadata:
        font_path = font['font_dir']
        if not os.path.exists(font_path):
            print(f"Warning: Font file {font_path} does not exist. Skipping.")
            continue
        # Copy the font file to fonts directory
        os.makedirs(os.path.join(os.path.dirname(html_file), "fonts"), exist_ok=True)
        target_font_path = os.path.join(os.path.dirname(html_file), "fonts", os.path.basename(font_path))
        if not os.path.exists(target_font_path):
            with open(font_path, "rb") as src_f, open(target_font_path, "wb") as dst_f:
                dst_f.write(src_f.read())
            print(f"Copied font {font_path} to {target_font_path}")
        font_path = target_font_path
        # Find the font path relative to this file
        font_rel_path = os.path.relpath(font_path, os.path.dirname(html_file)).replace("\\", "/")
        
        preload_lines +=f'    <link rel="preload" href="{font_rel_path}" as="font" type="font/ttf" crossorigin>' + "\n"
        import_lines += f"        @font-face {{ font-family: '{os.path.splitext(os.path.basename(font_path))[0]}'; src: url('{font_rel_path}'); font-weight: 400; font-style: normal; }}\n"
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
    print(f"Populated {html_file} with {len(fonts_metadata)} fonts from {font_dir}.")
    return html_file


if __name__ == "__main__":
    # Copy the template file:
    html_file = copy_html_file_from_template()
    # Populate the html file with fonts:
    populate_html_with_fonts(html_file)
    print("Setup complete.")

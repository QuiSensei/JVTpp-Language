import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import os
import sys

# -------------------------------------------------
# Locate compiler (jvtpp.exe)
# -------------------------------------------------
def resource_path(relative_path):
    """ Get absolute path to resource (for PyInstaller) """
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)


COMPILER_PATH = resource_path(
    os.path.join("JVT++ (Flex and Bison)", "jvtpp.exe")
)


# -------------------------------------------------
# IDE Class
# -------------------------------------------------
class JVTppIDE(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("JVT++ IDE")
        self.geometry("1000x650")

        self.filename = None

        self.create_menu()
        self.create_editor()
        self.create_output()

    # ---------------- MENU ----------------
    def create_menu(self):
        menubar = tk.Menu(self)

        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New", command=self.new_file)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save", command=self.save_file)
        file_menu.add_command(label="Save As", command=self.save_as)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.quit)

        run_menu = tk.Menu(menubar, tearoff=0)
        run_menu.add_command(label="Run", command=self.run_code)

        menubar.add_cascade(label="File", menu=file_menu)
        menubar.add_cascade(label="Run", menu=run_menu)

        self.config(menu=menubar)

    # ---------------- EDITOR ----------------
    def create_editor(self):
        editor_frame = tk.Frame(self)
        editor_frame.pack(fill=tk.BOTH, expand=True)

        self.editor = tk.Text(
            editor_frame,
            wrap=tk.NONE,
            font=("Consolas", 12),
            undo=True
        )

        y_scroll = tk.Scrollbar(editor_frame, orient=tk.VERTICAL, command=self.editor.yview)
        x_scroll = tk.Scrollbar(editor_frame, orient=tk.HORIZONTAL, command=self.editor.xview)

        self.editor.configure(
            yscrollcommand=y_scroll.set,
            xscrollcommand=x_scroll.set
        )

        self.editor.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        y_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        x_scroll.pack(side=tk.BOTTOM, fill=tk.X)

    # ---------------- OUTPUT ----------------
    def create_output(self):
        output_frame = tk.LabelFrame(self, text="Output")
        output_frame.pack(fill=tk.BOTH)

        self.output = tk.Text(
            output_frame,
            height=10,
            bg="black",
            fg="lime",
            font=("Consolas", 11)
        )
        self.output.pack(fill=tk.BOTH, expand=True)

    # ---------------- FILE OPS ----------------
    def new_file(self):
        self.editor.delete(1.0, tk.END)
        self.filename = None
        self.title("JVT++ IDE")

    def open_file(self):
        file = filedialog.askopenfilename(
            filetypes=[("JVT++ Files", "*.jvt"), ("All Files", "*.*")]
        )
        if file:
            with open(file, "r") as f:
                self.editor.delete(1.0, tk.END)
                self.editor.insert(tk.END, f.read())
            self.filename = file
            self.title(f"JVT++ IDE - {os.path.basename(file)}")

    def save_file(self):
        if self.filename:
            with open(self.filename, "w") as f:
                f.write(self.editor.get(1.0, tk.END))
        else:
            self.save_as()

    def save_as(self):
        file = filedialog.asksaveasfilename(
            defaultextension=".jvt",
            filetypes=[("JVT++ Files", "*.jvt")]
        )
        if file:
            self.filename = file
            self.save_file()
            self.title(f"JVT++ IDE - {os.path.basename(file)}")

    # ---------------- RUN ----------------
    def run_code(self):
        self.output.delete(1.0, tk.END)

        if not os.path.exists(COMPILER_PATH):
            messagebox.showerror("Error", "Compiler (jvtpp.exe) not found!")
            return

        code = self.editor.get(1.0, tk.END)

        temp_file = "temp.jvt"
        with open(temp_file, "w") as f:
            f.write(code)

        try:
            process = subprocess.run(
                [COMPILER_PATH, temp_file],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            if process.stdout:
                self.output.insert(tk.END, process.stdout)

            if process.stderr:
                self.output.insert(tk.END, "\nERRORS:\n")
                self.output.insert(tk.END, process.stderr)

        except Exception as e:
            self.output.insert(tk.END, str(e))


# -------------------------------------------------
# Run App
# -------------------------------------------------
if __name__ == "__main__":
    app = JVTppIDE()
    app.mainloop()

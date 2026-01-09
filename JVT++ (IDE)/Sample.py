import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import subprocess
import os

# ------------------------------------
# Compiler path
# ------------------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

COMPILER = os.path.join(
    BASE_DIR,
    "..",
    "JVT++ (Flex and Bison)",
    "jvtpp.exe"
)

MACHINE_CODE = os.path.join(
    BASE_DIR,
    "..",
    "JVT++ (Machine-Code)",
    "Machine-code.exe"
)

# ------------------------------------
# LineNumberedText Widget
# ------------------------------------
class LineNumberedText(tk.Frame):
    def __init__(self, master, **kwargs):
        super().__init__(master)
        self.text = tk.Text(self, **kwargs)
        self.linenumbers = tk.Text(self, width=4, padx=4, takefocus=0, border=0,
                                   background='lightgray', state='disabled', wrap='none')
        self.linenumbers.pack(side="left", fill="y")
        self.text.pack(side="right", fill="both", expand=True)
        self.text.bind("<KeyRelease>", self.update_line_numbers)
        self.text.bind("<MouseWheel>", self.sync_scroll)
        self.text.bind("<Button-1>", self.update_line_numbers)
        self.text.bind("<ButtonRelease-1>", self.update_line_numbers)
        self.text.bind("<Return>", self.update_line_numbers)
        self.update_line_numbers()

    def update_line_numbers(self, event=None):
        self.linenumbers.config(state='normal')
        self.linenumbers.delete(1.0, tk.END)
        line_count = self.text.index('end-1c').split('.')[0]
        numbers = "\n".join(str(i) for i in range(1, int(line_count) + 1))
        self.linenumbers.insert(1.0, numbers)
        self.linenumbers.config(state='disabled')

    def sync_scroll(self, event):
        self.linenumbers.yview_moveto(self.text.yview()[0])

# ------------------------------------
# Main App
# ------------------------------------
class JVTppIDE(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("JVT++ IDE")
        self.geometry("900x600")

        self.files = {}  # tab -> filename

        self.create_menu()
        self.create_tabs()
        self.create_output()

        # Bind F5 to run code
        self.bind("<F5>", lambda event: self.run_code())

        self.new_tab()

    # ---------- MENU ----------
    def create_menu(self):
        menu = tk.Menu(self)

        file_menu = tk.Menu(menu, tearoff=0)
        file_menu.add_command(label="New", command=self.new_tab)
        file_menu.add_command(label="Open", command=self.open_file)
        file_menu.add_command(label="Save", command=self.save_file)
        menu.add_cascade(label="File", menu=file_menu)

        run_menu = tk.Menu(menu, tearoff=0)
        run_menu.add_command(label="Run", command=self.run_code)
        menu.add_cascade(label="Run", menu=run_menu)

        self.config(menu=menu)

    # ---------- TABS ----------
    def create_tabs(self):
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill=tk.BOTH, expand=True)

    def new_tab(self, content=""):
        frame = tk.Frame(self.notebook)

        editor = LineNumberedText(frame, font=("Consolas", 12))
        editor.text.insert(tk.END, content)
        editor.pack(fill=tk.BOTH, expand=True)

        self.notebook.add(frame, text="Untitled")
        self.notebook.select(frame)
        self.files[frame] = None

        # Add close button
        self.add_tab_close_button(frame)

    def add_tab_close_button(self, tab_frame):
        tab_id = self.notebook.index(tab_frame)
        tab_text = self.notebook.tab(tab_id, option="text") + "  ✕"
        self.notebook.tab(tab_id, text=tab_text)
        self.notebook.bind("<Button-1>", self.on_tab_click)

    def on_tab_click(self, event):
        x, y = event.x, event.y
        element = self.notebook.identify(event.x, event.y)
        if "label" in element:
            index = self.notebook.index("@%d,%d" % (x, y))
            tab_text = self.notebook.tab(index, "text")
            if tab_text.endswith("✕"):
                self.notebook.forget(index)
                tab_frame = list(self.files.keys())[index]
                del self.files[tab_frame]

    def current_tab(self):
        return self.notebook.nametowidget(self.notebook.select())

    def current_text(self):
        return self.current_tab().winfo_children()[0].text

    # ---------- FILE OPS ----------
    def open_file(self):
        file = filedialog.askopenfilename(
            filetypes=[("JVT++ Files", "*.jvt"), ("All Files", "*.*")]
        )
        if not file:
            return

        with open(file, "r") as f:
            content = f.read()

        self.new_tab(content)
        tab = self.current_tab()
        self.files[tab] = file
        self.notebook.tab(tab, text=os.path.basename(file) + "  ✕")

    def save_file(self):
        tab = self.current_tab()
        text = self.current_text()

        filename = self.files.get(tab)

        if not filename:
            filename = filedialog.asksaveasfilename(
                defaultextension=".jvt",
                filetypes=[("JVT++ Files", "*.jvt")]
            )
            if not filename:
                return
            self.files[tab] = filename
            self.notebook.tab(tab, text=os.path.basename(filename) + "  ✕")

        with open(filename, "w") as f:
            f.write(text.get(1.0, tk.END))
        
        return True  # Indicate successful save

    # ---------- OUTPUT ----------
    def create_output(self):
        frame = tk.LabelFrame(self, text="Output")
        frame.pack(fill=tk.BOTH)

        self.output = tk.Text(frame, height=8, bg="black", fg="white")
        self.output.pack(fill=tk.BOTH, expand=True)

    # ---------- RUN ----------
    def run_code(self):
        self.output.delete(1.0, tk.END)

        # Check if compiler exists
        if not os.path.exists(COMPILER):
            self.output.insert(tk.END, "ERROR: jvtpp.exe not found\n")
            return
        
        # Check if machine code executable exists
        if not os.path.exists(MACHINE_CODE):
            self.output.insert(tk.END, "ERROR: Machine-code.exe not found\n")
            return

        tab = self.current_tab()
        filename = self.files.get(tab)

        # Save the file first before running
        if not filename:
            # File hasn't been saved yet, prompt user to save
            response = messagebox.askyesno(
                "Save File",
                "File needs to be saved before running. Save now?"
            )
            if response:
                if not self.save_file():
                    self.output.insert(tk.END, "ERROR: File not saved. Run cancelled.\n")
                    return
                filename = self.files.get(tab)
            else:
                self.output.insert(tk.END, "ERROR: File must be saved before running.\n")
                return
        else:
            # File has been saved before, save current changes
            if not self.save_file():
                self.output.insert(tk.END, "ERROR: Could not save file.\n")
                return

        # Get the directory where the .jvt file is saved
        file_dir = os.path.dirname(filename)

        # Run the compiler with the saved file
        result = subprocess.run(
            [COMPILER, filename],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        self.output.insert(tk.END, result.stdout)
        self.output.insert(tk.END, result.stderr)

        # Run Machine-code.exe in the background in the same directory as the .jvt file
        subprocess.Popen(
            [MACHINE_CODE],
            cwd=file_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )


# ------------------------------------
# Run app
# ------------------------------------
if __name__ == "__main__":
    app = JVTppIDE()
    app.mainloop()
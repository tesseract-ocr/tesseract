"""Universal Tessdata Path Finder for Windows and Linux systems."""
import os
import shutil
import subprocess
import glob
import platform

try:
    import winreg
except ImportError:
    winreg = None

def get_tessdata_paths(verbose=False):
    """Find all tessdata directory paths on the system (Windows and Linux).
    
    Args:
        verbose (bool): If True, prints search progress
    
    Returns:
        list: List of tessdata directory paths, empty if none found
    """
    paths = []
    
    if verbose:
        print("Searching for tessdata paths...")
    
    # Detect OS
    current_os = platform.system().lower()
    if verbose:
        print(f"Detected OS: {current_os}")
    
    if current_os == 'windows':
        paths = _find_tessdata_windows(verbose)
    else:
        paths = _find_tessdata_linux(verbose)
    
    if verbose:
        print(f"Total found: {len(paths)} tessdata paths")
    
    return paths

def get_primary_tessdata_path(verbose=False):
    """Get the primary (first/best) tessdata directory path.
    
    Args:
        verbose (bool): If True, prints search progress
        
    Returns:
        str: Primary tessdata path, None if not found
    """
    paths = get_tessdata_paths(verbose=verbose)
    return paths[0] if paths else None

def _find_tessdata_windows(verbose=False):
    """Find tessdata paths on Windows systems."""
    paths = []
    
    # 1. Environment variable
    env_path = os.environ.get('TESSDATA_PREFIX')
    if (env_path and os.path.isdir(env_path) and 
        _is_valid_tessdata_dir(env_path)):
        paths.append(env_path)
        if verbose:
            print(f"Found via ENV: {env_path}")
    
    # 2. Windows Registry check
    registry_paths = _get_tessdata_from_registry()
    for path in registry_paths:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found via registry: {path}")
    
    # 3. From tesseract binary locations
    tess_binaries = _find_tesseract_binaries_windows()
    for tess_bin in tess_binaries:
        bin_dir = os.path.dirname(os.path.realpath(tess_bin))
        
        # Windows-specific relative paths
        relative_paths = [
            "tessdata",
            "../tessdata",
            "../../tessdata",
            "../share/tessdata",
            "../share/tesseract-ocr/tessdata"
        ]
        
        for rel_path in relative_paths:
            path = os.path.normpath(os.path.join(bin_dir, rel_path))
            if (os.path.isdir(path) and _is_valid_tessdata_dir(path) and 
                path not in paths):
                paths.append(path)
                if verbose:
                    print(f"Found via binary: {path}")
    
    # 4. Common Windows installation paths
    common_windows_paths = [
        "C:\\Program Files\\Tesseract-OCR\\tessdata",
        "C:\\Program Files (x86)\\Tesseract-OCR\\tessdata",
        "C:\\Tesseract-OCR\\tessdata",
        "C:\\tools\\tesseract\\tessdata",
        "C:\\tesseract\\tessdata",
        f"{os.environ.get('LOCALAPPDATA', '')}\\Tesseract-OCR\\tessdata",
        f"{os.environ.get('PROGRAMFILES', '')}\\Tesseract-OCR\\tessdata",
        f"{os.environ.get('PROGRAMFILES(X86)', '')}\\Tesseract-OCR\\tessdata"
    ]
    
    for path in common_windows_paths:
        if (path and os.path.isdir(path) and _is_valid_tessdata_dir(path) and 
            path not in paths):
            paths.append(path)
            if verbose:
                print(f"Found in common location: {path}")
    
    # 5. Windows filesystem search
    if verbose:
        print("Searching Windows drives...")
    
    search_paths = _comprehensive_windows_search()
    for path in search_paths:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found via search: {path}")
    
    # 6. Check user directories
    user_paths = _check_windows_user_locations()
    for path in user_paths:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found in user dir: {path}")
    
    return paths

def _find_tessdata_linux(verbose=False):
    """Find tessdata paths on Linux systems."""
    paths = []
    
    # 1. Environment variable
    env_path = os.environ.get('TESSDATA_PREFIX')
    if (env_path and os.path.isdir(env_path) and 
        _is_valid_tessdata_dir(env_path)):
        paths.append(env_path)
        if verbose:
            print(f"Found via ENV: {env_path}")
    
    # 2. From all tesseract binary locations
    tess_binaries = _find_tesseract_binaries_linux()
    for tess_bin in tess_binaries:
        bin_dir = os.path.dirname(os.path.realpath(tess_bin))
        
        relative_paths = [
            "../share/tesseract-ocr/tessdata",
            "../share/tesseract/tessdata", 
            "../../share/tesseract-ocr/tessdata",
            "../../share/tesseract/tessdata",
            "../tessdata",
            "tessdata"
        ]
        
        for rel_path in relative_paths:
            path = os.path.normpath(os.path.join(bin_dir, rel_path))
            if (os.path.isdir(path) and _is_valid_tessdata_dir(path) and 
                path not in paths):
                paths.append(path)
                if verbose:
                    print(f"Found via binary: {path}")
    
    # 3. Direct from tesseract
    tessdata_from_binary = _get_tessdata_from_tesseract()
    for path in tessdata_from_binary:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found via tesseract: {path}")
    
    # 4. Comprehensive filesystem search
    found_paths = _comprehensive_linux_search()
    for path in found_paths:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found via search: {path}")
    
    # 5. User-specific locations
    user_paths = _check_linux_user_locations()
    for path in user_paths:
        if path not in paths:
            paths.append(path)
            if verbose:
                print(f"Found in user dir: {path}")
    
    return paths

def _get_tessdata_from_registry():
    """Get tessdata path from Windows registry."""
    paths = []
    
    if winreg is None:
        return paths
    
    try:
        # Common registry locations for Tesseract
        registry_keys = [
            (winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Tesseract-OCR"),
            (winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Tesseract-OCR"),
            (winreg.HKEY_CURRENT_USER, "SOFTWARE\\Tesseract-OCR")
        ]
        
        for hkey, subkey in registry_keys:
            try:
                with winreg.OpenKey(hkey, subkey) as key:
                    install_path, _ = winreg.QueryValueEx(key, "InstallPath")
                    tessdata_path = os.path.join(install_path, "tessdata")
                    if (os.path.isdir(tessdata_path) and 
                        _is_valid_tessdata_dir(tessdata_path)):
                        paths.append(tessdata_path)
            except (FileNotFoundError, OSError):
                continue
    except ImportError:
        # winreg not available (not Windows)
        pass
    
    return paths

def _find_tesseract_binaries_windows():
    """Find tesseract binaries on Windows."""
    binaries = []
    
    # Method 1: which/where command
    tess_bin = shutil.which("tesseract")
    if tess_bin:
        binaries.append(tess_bin)
    
    tess_bin_exe = shutil.which("tesseract.exe")
    if tess_bin_exe and tess_bin_exe not in binaries:
        binaries.append(tess_bin_exe)
    
    # Method 2: Common Windows locations
    common_paths = [
        "C:\\Program Files\\Tesseract-OCR\\tesseract.exe",
        "C:\\Program Files (x86)\\Tesseract-OCR\\tesseract.exe",
        "C:\\Tesseract-OCR\\tesseract.exe",
        "C:\\tools\\tesseract\\tesseract.exe"
    ]
    
    for path in common_paths:
        if os.path.isfile(path) and path not in binaries:
            binaries.append(path)
    
    # Method 3: Search Program Files
    program_dirs = [
        os.environ.get('PROGRAMFILES', ''),
        os.environ.get('PROGRAMFILES(X86)', ''),
        'C:\\tools',
        'C:\\'
    ]
    
    for prog_dir in program_dirs:
        if prog_dir and os.path.exists(prog_dir):
            try:
                found = _search_for_binaries_windows(prog_dir, "tesseract.exe")
                binaries.extend([b for b in found if b not in binaries])
            except:
                continue
    
    return binaries

def _find_tesseract_binaries_linux():
    """Find tesseract binaries on Linux."""
    binaries = []
    
    # Method 1: which command
    tess_bin = shutil.which("tesseract")
    if tess_bin:
        binaries.append(tess_bin)
    
    # Method 2: Common locations
    common_paths = [
        "/usr/bin/tesseract",
        "/usr/local/bin/tesseract", 
        "/opt/tesseract/bin/tesseract",
        "/bin/tesseract"
    ]
    
    for path in common_paths:
        if (os.path.isfile(path) and os.access(path, os.X_OK) and 
            path not in binaries):
            binaries.append(path)
    
    # Method 3: Search filesystem
    search_dirs = ["/usr", "/opt", "/usr/local"]
    for search_dir in search_dirs:
        if os.path.exists(search_dir):
            try:
                found = _search_for_binaries_linux(search_dir, "tesseract")
                binaries.extend([b for b in found if b not in binaries])
            except:
                continue
    
    return binaries

def _search_for_binaries_windows(root_dir, binary_name, max_depth=3):
    """Search for binary files on Windows."""
    found = []
    
    def _search(current_dir, depth):
        if depth > max_depth:
            return
        
        try:
            for entry in os.listdir(current_dir):
                entry_path = os.path.join(current_dir, entry)
                
                if (os.path.isfile(entry_path) and 
                    entry.lower() == binary_name.lower()):
                    found.append(entry_path)
                elif os.path.isdir(entry_path):
                    _search(entry_path, depth + 1)
        except (PermissionError, OSError):
            pass
    
    _search(root_dir, 0)
    return found

def _search_for_binaries_linux(root_dir, binary_name, max_depth=3):
    """Search for binary files on Linux."""
    found = []
    
    def _search(current_dir, depth):
        if depth > max_depth:
            return
        
        try:
            for entry in os.listdir(current_dir):
                entry_path = os.path.join(current_dir, entry)
                
                if os.path.isfile(entry_path) and entry == binary_name:
                    if os.access(entry_path, os.X_OK):
                        found.append(entry_path)
                elif (os.path.isdir(entry_path) and 
                      not os.path.islink(entry_path)):
                    if any(bin_dir in entry for bin_dir in ['bin', 'sbin']):
                        _search(entry_path, depth + 1)
        except (PermissionError, OSError):
            pass
    
    _search(root_dir, 0)
    return found

def _comprehensive_windows_search():
    """Search Windows filesystem for tessdata."""
    found_paths = []
    
    # Get all available drives
    drives = []
    for letter in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
        drive = f"{letter}:\\"
        if os.path.exists(drive):
            drives.append(drive)
    
    # Search common locations on each drive
    for drive in drives:
        search_dirs = [
            os.path.join(drive, "Program Files"),
            os.path.join(drive, "Program Files (x86)"),
            drive  # Root of drive
        ]
        
        for search_dir in search_dirs:
            if os.path.exists(search_dir):
                try:
                    paths = _search_tessdata_recursive_windows(
                        search_dir, max_depth=4)
                    found_paths.extend(paths)
                except (PermissionError, OSError):
                    continue
    
    return list(set(found_paths))

def _comprehensive_linux_search():
    """Search Linux filesystem for tessdata"""
    found_paths = []
    search_roots = ["/usr", "/usr/local", "/opt", "/var"]
    
    for root in search_roots:
        if os.path.exists(root):
            try:
                paths = _search_tessdata_recursive_linux(root, max_depth=5)
                found_paths.extend(paths)
                
                # Glob patterns
                patterns = [f"{root}/**/tessdata", f"{root}/share/**/tessdata"]
                for pattern in patterns:
                    try:
                        matches = glob.glob(pattern, recursive=True)
                        for match in matches:
                            if os.path.isdir(match) and _is_valid_tessdata_dir(match):
                                found_paths.append(match)
                    except:
                        continue
            except:
                continue
    
    return list(set(found_paths))

def _search_tessdata_recursive_windows(root_dir, max_depth=4):
    """Recursive search for tessdata on Windows."""
    found = []
    
    def _search(current_dir, depth):
        if depth > max_depth:
            return
        
        try:
            if os.path.basename(current_dir).lower() == 'tessdata':
                if _is_valid_tessdata_dir(current_dir):
                    found.append(current_dir)
            
            for entry in os.listdir(current_dir):
                entry_path = os.path.join(current_dir, entry)
                if os.path.isdir(entry_path):
                    # Skip system directories
                    skip_dirs = {
                        'windows', 'system32', 'syswow64', '$recycle.bin'
                    }
                    if entry.lower() not in skip_dirs:
                        _search(entry_path, depth + 1)
        except (PermissionError, OSError):
            pass
    
    _search(root_dir, 0)
    return found

def _search_tessdata_recursive_linux(root_dir, max_depth=5):
    """Recursive search for tessdata on Linux"""
    found = []
    
    def _search(current_dir, depth):
        if depth > max_depth:
            return
        
        try:
            if os.path.basename(current_dir) == 'tessdata':
                if _is_valid_tessdata_dir(current_dir):
                    found.append(current_dir)
            
            for entry in os.listdir(current_dir):
                entry_path = os.path.join(current_dir, entry)
                if os.path.isdir(entry_path) and not os.path.islink(entry_path):
                    skip_dirs = {'proc', 'sys', 'dev', 'run', 'tmp'}
                    if entry not in skip_dirs:
                        _search(entry_path, depth + 1)
        except (PermissionError, OSError):
            pass
    
    _search(root_dir, 0)
    return found

def _check_windows_user_locations():
    """Check Windows user-specific locations."""
    paths = []
    user_home = os.path.expanduser("~")
    appdata = os.environ.get('APPDATA', '')
    localappdata = os.environ.get('LOCALAPPDATA', '')
    
    locations = [
        os.path.join(user_home, "tessdata"),
        os.path.join(user_home, "Tesseract-OCR", "tessdata"),
        os.path.join(appdata, "Tesseract-OCR", "tessdata") if appdata else "",
        os.path.join(localappdata, "Tesseract-OCR", "tessdata") if localappdata else ""
    ]
    
    for location in locations:
        if (location and os.path.isdir(location) and 
            _is_valid_tessdata_dir(location)):
            paths.append(location)
    
    return paths

def _check_linux_user_locations():
    """Check Linux user-specific locations."""
    paths = []
    home = os.path.expanduser("~")
    
    locations = [
        f"{home}/.local/share/tesseract/tessdata",
        f"{home}/.tesseract/tessdata",
        f"{home}/tesseract/tessdata"
    ]
    
    for location in locations:
        if os.path.isdir(location) and _is_valid_tessdata_dir(location):
            paths.append(location)
    
    return paths

def _get_tessdata_from_tesseract():
    """Get tessdata paths from tesseract binary (Linux/Unix)"""
    paths = []
    
    if platform.system().lower() == 'windows':
        binaries = _find_tesseract_binaries_windows()
    else:
        binaries = _find_tesseract_binaries_linux()
    
    for binary in binaries:
        try:
            result = subprocess.run([binary, "--print-parameters"], 
                                  capture_output=True, text=True, timeout=5)
            for line in result.stdout.split('\n'):
                if 'tessdata' in line.lower():
                    parts = line.split()
                    for part in parts:
                        if ('tessdata' in part and os.path.isdir(part) and 
                            _is_valid_tessdata_dir(part)):
                            paths.append(part)
        except:
            continue
    
    return list(set(paths))

def _is_valid_tessdata_dir(path):
    """Check if directory contains tessdata files"""
    try:
        files = os.listdir(path)
        return any(f.endswith('.traineddata') for f in files)
    except:
        return False

# Example usage
if __name__ == "__main__":
    print(f"Running on: {platform.system()}")
    
    # Get all paths
    tessdata_paths = get_tessdata_paths(verbose=True)
    print(f"\nAll tessdata paths: {tessdata_paths}")
    
    # Get primary path
    primary_path = get_primary_tessdata_path()
    print(f"Primary tessdata path: {primary_path}")

"""Train the small handwritten-digit classifier used for order-quantity cells, then export it as plain numpy weights so the
server needs no torch at runtime.   python scripts/train_digits.py [out.npz]   (needs torch; downloads MNIST once, ~11 MB)"""
import gzip
import math
import struct
import sys
import urllib.request
from pathlib import Path

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

BASE = "https://ossci-datasets.s3.amazonaws.com/mnist/"
CACHE = Path(__file__).resolve().parent / ".mnist"


def fetch(name):
    CACHE.mkdir(exist_ok=True)
    p = CACHE / name
    if not p.exists():
        urllib.request.urlretrieve(BASE + name, p)
    return gzip.open(p).read()


def images(name):
    b = fetch(name)
    _, n, r, c = struct.unpack(">IIII", b[:16])
    return np.frombuffer(b, np.uint8, offset=16).reshape(n, 1, r, c).astype(np.float32) / 255


def labels(name):
    return np.frombuffer(fetch(name), np.uint8, offset=8).astype(np.int64)


class Net(nn.Module):
    def __init__(self):
        super().__init__()
        self.c1, self.c2 = nn.Conv2d(1, 12, 3), nn.Conv2d(12, 24, 3)
        self.f1, self.f2 = nn.Linear(24 * 5 * 5, 64), nn.Linear(64, 10)

    def forward(self, x):
        x = F.max_pool2d(F.relu(self.c1(x)), 2)      # 28 -> 26 -> 13
        x = F.max_pool2d(F.relu(self.c2(x)), 2)      # 13 -> 11 -> 5
        return self.f2(F.relu(self.f1(x.flatten(1))))


def augment(x):
    """Handwriting on forms is slanted, thick or thin and off-centre: randomise all of that."""
    n = x.shape[0]
    ang = (torch.rand(n) - 0.5) * 2 * math.radians(15)
    shear = (torch.rand(n) - 0.5) * 0.7
    sc = 0.8 + torch.rand(n) * 0.35
    tx, ty = (torch.rand(n) - 0.5) * 0.25, (torch.rand(n) - 0.5) * 0.25
    th = torch.zeros(n, 2, 3)
    th[:, 0, 0], th[:, 0, 1], th[:, 1, 0], th[:, 1, 1] = torch.cos(ang) / sc, -torch.sin(ang) / sc + shear, torch.sin(ang) / sc, torch.cos(ang) / sc
    th[:, 0, 2], th[:, 1, 2] = tx, ty
    x = F.grid_sample(x, F.affine_grid(th, x.shape, align_corners=False), align_corners=False)
    thick = torch.rand(n) < 0.35
    x[thick] = F.max_pool2d(x[thick], 3, 1, 1)
    thin = (torch.rand(n) < 0.2) & ~thick
    x[thin] = -F.max_pool2d(-x[thin], 3, 1, 1) * 0.0 + x[thin] * (x[thin] > 0.55)
    return x


if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "storage/models/digits.npz")
    torch.manual_seed(0)
    xtr, ytr = torch.tensor(images("train-images-idx3-ubyte.gz")), torch.tensor(labels("train-labels-idx1-ubyte.gz"))
    xte, yte = torch.tensor(images("t10k-images-idx3-ubyte.gz")), torch.tensor(labels("t10k-labels-idx1-ubyte.gz"))
    net, opt = Net(), None
    opt = torch.optim.Adam(net.parameters(), 2e-3)
    sched = torch.optim.lr_scheduler.OneCycleLR(opt, 4e-3, total_steps=8 * (len(xtr) // 128 + 1))
    for ep in range(8):
        perm = torch.randperm(len(xtr))
        net.train()
        for i in range(0, len(xtr), 128):
            idx = perm[i:i + 128]
            loss = F.cross_entropy(net(augment(xtr[idx].clone())), ytr[idx])
            opt.zero_grad(); loss.backward(); opt.step(); sched.step()
        net.eval()
        with torch.no_grad():
            acc = (net(xte).argmax(1) == yte).float().mean().item()
            accaug = (net(augment(xte.clone())).argmax(1) == yte).float().mean().item()
        print(f"epoch {ep + 1}: clean test acc {acc:.4f}  augmented test acc {accaug:.4f}", flush=True)
    out.parent.mkdir(parents=True, exist_ok=True)
    np.savez(out, **{k: v.detach().numpy() for k, v in net.state_dict().items()})
    print("saved", out, out.stat().st_size, "bytes")

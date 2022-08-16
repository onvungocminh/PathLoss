import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import cv2
import MBD
from loss import *
import scipy.io
from PIL import Image
import pdb


img_file = 'images/prediction.png'
img = Image.open(img_file)
img = np.array(img, dtype=np.float32)/255
seed_file = 'images/seed.mat'
seed = scipy.io.loadmat(seed_file)["seed"]
boundary_file = 'images/boundary.mat'
boundary = scipy.io.loadmat(boundary_file)["boundary"]	
contour_file = 'images/gt.png'
contour = Image.open(contour_file)
contour = contour.convert('L')
contour = np.array(contour)/255.
contour = contour.astype(np.uint8)


img = torch.from_numpy(img.copy()).float()
img = img.unsqueeze(axis=0)
contour = torch.from_numpy(np.array([contour])).float()
seed = torch.from_numpy(np.array([seed])).squeeze(axis=0).float()
boundary = torch.from_numpy(np.array([boundary])).squeeze(axis=0).float()

img, seed, boundary = img.cuda(),  seed.cuda(), boundary.cuda()

PathLoss = DAHU_loss(img, seed, boundary)
print(PathLoss)

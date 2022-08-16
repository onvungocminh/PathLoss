import numpy as np
from scipy.ndimage.morphology import distance_transform_edt
from skimage.segmentation import find_boundaries
from scipy import ndimage

import torch
import torch.nn as nn
import torch.nn.functional as F
import matplotlib.pyplot as plt
import cv2
from scipy import ndimage
import MBD


import pdb

class DiceBCELoss(nn.Module):
    def __init__(self, weight=None, size_average=False):
        super(DiceBCELoss, self).__init__()

    def forward(self, inputs, targets, smooth=1):
        #comment out if your model contains a sigmoid or equivalent activation layer
        inputs = torch.sigmoid(inputs)       
        
        #flatten label and prediction tensors
        inputs = inputs.view(-1)
        targets = targets.view(-1)
        
        intersection = (inputs * targets).sum()                            
        dice_loss = 1 - (2.*intersection + smooth)/(inputs.sum() + targets.sum() + smooth)  
        BCE = F.binary_cross_entropy(inputs, targets, reduction='mean')
        Dice_BCE = BCE + dice_loss
        
        return Dice_BCE

def cross_entropy_loss2d(inputs, targets, cuda=True, balance=1.1):
    """
    :param inputs: inputs is a 4 dimensional data nx1xhxw
    :param targets: targets is a 3 dimensional data nx1xhxw
    :return:
    """
    n, c, h, w = inputs.size()
    weights = np.zeros((n, c, h, w))
    for i in range(n):
        t = targets[i, :, :].cpu().data.numpy()
        pos = (t == 1).sum()
        neg = (t == 0).sum()
        valid = neg + pos
        weights[i, t == 1] = neg * 1. / valid
        weights[i, t == 0] = pos * balance / valid
    weights = torch.Tensor(weights)
    if cuda:
        weights = weights.cuda()
    loss = nn.BCEWithLogitsLoss(weights, reduction='mean')(inputs, targets)
    return loss

def DAHU_loss(inputs, seed, boundary):
    '''
    inputs : prediction image
    seed: seeding pixel (geodesic center of the connected component)
    boundary: boundary of the connected component
    '''
    EPM_s = inputs.squeeze()

    seed = seed.squeeze(axis = 0).squeeze(axis = 0)
    boundary = boundary.squeeze(axis=0).squeeze(axis = 0)

    d, _, _ = seed.shape
    loss_total = 0

    EPM_s_clone = EPM_s.clone()
    EPM_s_clone = np.array((EPM_s_clone*255).detach().cpu().numpy()).astype(np.uint8)


    for i in range(d):
        loss = 0
        seed_s, boundary_s = seed[i,:,:], boundary[i,:,:]

        # transform tensor to array to find the shortest path in C++
        max_range = torch.max(seed_s).type(torch.uint8)
        seed_s = np.array(seed_s.detach().cpu().numpy()).astype(np.uint8)    
        destination = np.array(seed_s).astype(np.uint8)

        if max_range>1:
            start = np.array(seed_s == 1).astype(np.uint8)*255
            max_value_array = torch.zeros(max_range-1).cuda()
            max_value_array_gt = torch.ones(max_range-1).cuda()    

            # find the shortest path using djikstra-like algorithm
            shortest_path = MBD.geodesic_shortest_all(EPM_s_clone,start,destination)
            shortest_path = torch.from_numpy(np.array([shortest_path])).cuda()
            
            for j in range(2, max_range +1):
                path_tmp = shortest_path == j
                
                # get the value of the intersection between the shortest path 
                # and the boundary of the connected component 
                maximum_value = torch.max(EPM_s * path_tmp * boundary_s)
                max_value_array[j-2] = maximum_value

            # compute the MSE loss function
            loss = loss + nn.MSELoss()(max_value_array, max_value_array_gt)     
            # torch.cuda.empty_cache()
        else:
            loss = torch.tensor(0).type(torch.float64)
        loss_total = loss_total + loss # Accumulate sum
    loss_total = loss_total / d

    return loss_total

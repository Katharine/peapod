//
//  KBViewController.m
//  pebbleremote
//
//  Created by Katharine Berry on 25/05/2013.
//  Copyright (c) 2013 Katharine Berry. All rights reserved.
//

#import "KBViewController.h"
#import "KBiPodRemote.h"

@interface KBViewController () {
    KBiPodRemote *remote;
}

@end

@implementation KBViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    remote = [[KBiPodRemote alloc] init];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end

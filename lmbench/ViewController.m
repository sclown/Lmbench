//
//  ViewController.m
//  lmbench
//
//  Created by d.kurkin on 28/08/2017.
//  Copyright © 2017 d.kurkin. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@property (strong) IBOutlet UILabel* label;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.label.text = @"Completed";
    // Do any additional setup after loading the view, typically from a nib.
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end

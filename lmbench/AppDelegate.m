//
//  AppDelegate.m
//  lmbench
//
//  Created by d.kurkin on 28/08/2017.
//  Copyright © 2017 d.kurkin. All rights reserved.
//

#import "AppDelegate.h"


void bw_file_rd(const char*);
void lat_fs(const char*);
void lat_ctx_random();
void lat_ctx();
void lat_thread();
void lat_sem();
void lat_mem_rd();
void lat_ops();
void bw_mem();

void lat_array();
void lat_string(const char*);

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    NSError *error;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
    
//    lat_mem_rd();
//    lat_fs([documentsDirectory UTF8String]);
//    bw_file_rd([documentsDirectory UTF8String]);
//    lat_ops();
//    bw_mem();

//    lat_thread();
    lat_ctx();
//    lat_ctx_random();
//    lat_sem();
//    lat_array();
//    lat_string([documentsDirectory UTF8String]);
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    exit(0);
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end

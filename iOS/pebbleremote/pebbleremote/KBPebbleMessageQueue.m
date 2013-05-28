//
//  KBPebbleMessageQueue.m
//  pebbleremote
//
//  Created by Katharine Berry on 27/05/2013.
//  Copyright (c) 2013 Katharine Berry. All rights reserved.
//

#import "KBPebbleMessageQueue.h"
#import <PebbleKit/PebbleKit.h>

@interface KBPebbleMessageQueue () {
    NSInteger failureCount;
}
- (void)sendRequest;
@end

@implementation KBPebbleMessageQueue

- (id)init
{
    self = [super init];
    if (self) {
        has_active_request = NO;
        queue = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)enqueue:(NSDictionary *)message {
    if(!_watch) {
        NSLog(@"No watch; discarding message.");
        return;
    }
    if(!message) return;
    @synchronized(queue) {
        //NSLog(@"Enqueued message: %@", message);
        [queue addObject:message];
        [self sendRequest];
    }
}

- (void)sendRequest {
    @synchronized(queue) {
        if(has_active_request) { NSLog(@"Request in-flight; stalling."); return; }
        if([queue count] == 0) { NSLog(@"Nothing in queue."); return; }
        if(![_watch isConnected]) {
            has_active_request = false;
            return;
        }
        //NSLog(@"Sending message.");
        has_active_request = YES;
        NSDictionary* message = [queue objectAtIndex:0];
        [_watch appMessagesPushUpdate:message onSent:^(PBWatch *watch, NSDictionary *update, NSError *error) {
            if(!error) {
                [queue removeObjectAtIndex:0];
                failureCount = 0;
                //NSLog(@"Successfully pushed: %@", message);
            } else {
                NSLog(@"Send failed; will retransmit.");
                NSLog(@"Error: %@", error);
                sleep(1);
                if(++failureCount > 5) {
                    [queue removeAllObjects];
                    NSLog(@"Aborting.");
                }
            }
            has_active_request = NO;
            //NSLog(@"Next message.");
            [self sendRequest];
        }];
    }
}

@end

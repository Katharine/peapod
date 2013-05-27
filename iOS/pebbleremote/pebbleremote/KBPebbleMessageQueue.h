//
//  KBPebbleMessageQueue.h
//  pebbleremote
//
//  Created by Katharine Berry on 27/05/2013.
//  Copyright (c) 2013 Katharine Berry. All rights reserved.
//

#import <Foundation/Foundation.h>

@class PBWatch;

@interface KBPebbleMessageQueue : NSObject {
    NSMutableArray *queue;
    BOOL has_active_request;
}

- (void)enqueue:(NSDictionary*)message;

@property (nonatomic, retain) PBWatch* watch;

@end

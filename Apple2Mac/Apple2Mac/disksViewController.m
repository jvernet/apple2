//
//  disksViewController.m
//  Apple2Mac
//
//  Created by Jerome Vernet on 31/12/2015.
//  Copyright © 2015 deadc0de.org. All rights reserved.
//

#import "disksViewController.h"
#import "common.h"

@implementation disksViewController



- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    
   
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    //self.path = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Disks"];
    //TODO: seed if a Subdirectory named Inbox exist
    //      if case, move all files to Document and delete Inbox
    //      Then browse...
    self.path = [paths objectAtIndex:0];
    NSLog(@"Path:%@",self.path);
    NSString *inboxPath;
    inboxPath = [self.path stringByAppendingPathComponent:@"Inbox"];
     NSLog(@"Path:%@",self.path);
    if ([[NSFileManager defaultManager] fileExistsAtPath:inboxPath]) //Check if Inbox exist
    {
        NSLog(@"Inbox Exist");
        NSArray *inboxContent = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:inboxPath error:NULL]; //Get content
        int Count;
        NSString *fileToMove,*fileDest;
        
        for (Count = 0; Count < (int)[inboxContent count]; Count++)
        {
            fileToMove=[inboxPath stringByAppendingPathComponent:[inboxContent objectAtIndex:Count]];
            fileDest=[self.path stringByAppendingPathComponent:[inboxContent objectAtIndex:Count]];
            NSLog(@"File %d: %@>%@", (Count + 1), fileToMove,fileDest);
            [[NSFileManager defaultManager] moveItemAtPath:fileToMove toPath:fileDest  error:nil];
        }
        
        NSError *error;
        if (![[NSFileManager defaultManager] removeItemAtPath:inboxPath error:&error])	//Delete it
        {
            NSLog(@"Delete directory error: %@", error);
        }
    }
    
    self._disks=[[NSFileManager defaultManager] contentsOfDirectoryAtPath:self.path error:NULL];
    
    // Connect data
    self.disk1Picker.dataSource = self;
    self.disk1Picker.delegate = self;
    self.disk2Picker.dataSource = self;
    self.disk2Picker.delegate = self;
    
    
}

// The number of columns of data
- (int)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

// The number of rows of data
- (int)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return self._disks.count;
    //_pickerData.cout;
}

// The data to return for the row and component (column) that's being passed in
- (NSString*)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return [self._disks objectAtIndex:row];
}

// Catpure the picker view selection
- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    // This method is triggered whenever the user makes a change to the picker selection.
    // The parameter named row and component represents what was selected.
    int drive=0;
    BOOL ro=YES;
    
    if(pickerView==self.disk1Picker)
    {
        drive=0;
        ro=self.diskAProtection.on;
    }
    if(pickerView==self.disk2Picker)
    {
     drive=1;
        ro=self.diskBProtection.on;
    }
    
    NSLog(@"Selected Row %d %@ %c", row,(NSString*)[self._disks objectAtIndex:row],ro);
    disk6_eject(drive);
    const char *errMsg = disk6_insert(drive, [[self.path stringByAppendingPathComponent:[self._disks objectAtIndex:row]] UTF8String], ro);
}

- (IBAction)unwindToMainViewController:(UIStoryboardSegue*)sender
{ }

-(IBAction)goodbye:(id)sender
{
    [self dismissViewControllerAnimated:YES completion:nil];
    cpu_resume();
}

- (void)dealloc {
    [_diskAProtection release];
    [_diskBProtection release];
    [super dealloc];
}
@end


//
//  RenameMPWorldViewController.h
//  minecraftpe
//
//  Created by rhino on 10/20/11.
//

#import "BaseDialogController.h"

@interface RenameMPWorldViewController : BaseDialogController<UITextFieldDelegate>
{
    IBOutlet UILabel* _labelName;

    IBOutlet UITextField* _textName;
}

- (IBAction)DismissKeyboard;

- (IBAction)Create;
- (IBAction)Cancel;

@end

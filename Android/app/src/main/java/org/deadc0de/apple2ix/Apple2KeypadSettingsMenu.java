/*
 * Apple // emulator for *nix
 *
 * This software package is subject to the GNU General Public License
 * version 3 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * Copyright 2015 Aaron Culliney
 *
 */

package org.deadc0de.apple2ix;

import android.view.View;
import android.widget.TextView;

import java.util.ArrayList;

import org.deadc0de.apple2ix.basic.R;
import org.json.JSONArray;

public class Apple2KeypadSettingsMenu extends Apple2AbstractMenu {

    private final static String TAG = "Apple2KeypadSettingsMenu";

    public final static int KEYREPEAT_NUM_CHOICES = Apple2Preferences.DECENT_AMOUNT_OF_CHOICES;

    public final static String PREF_KPAD_ROSETTE_CHAR_ARRAY = "kpAxisRosetteChars";
    public final static String PREF_KPAD_ROSETTE_SCAN_ARRAY = "kpAxisRosetteScancodes";
    public final static String PREF_KPAD_SWIPE_NORTH_CHAR = "kpSwipeNorthChar";
    public final static String PREF_KPAD_SWIPE_NORTH_SCAN = "kpSwipeNorthScancode";
    public final static String PREF_KPAD_SWIPE_SOUTH_CHAR = "kpSwipeSouthChar";
    public final static String PREF_KPAD_SWIPE_SOUTH_SCAN = "kpSwipeSouthScancode";
    public final static String PREF_KPAD_TOUCHDOWN_CHAR = "kpTouchDownChar";
    public final static String PREF_KPAD_TOUCHDOWN_SCAN = "kpTouchDownScancode";

    public final static int ROSETTE_SIZE = 9;

    public Apple2KeypadSettingsMenu(Apple2Activity activity) {
        super(activity);
    }

    @Override
    public final String[] allTitles() {
        return SETTINGS.titles(mActivity);
    }

    @Override
    public final IMenuEnum[] allValues() {
        return SETTINGS.values();
    }

    @Override
    public final boolean areAllItemsEnabled() {
        return true;
    }

    @Override
    public final boolean isEnabled(int position) {
        if (position < 0 || position >= SETTINGS.size) {
            throw new ArrayIndexOutOfBoundsException();
        }
        return true;
    }

    public enum KeypadPreset {
        ARROWS_SPACE {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_arrows_space);
            }

            @Override
            public void apply(Apple2Activity activity) {
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_UP, Apple2KeyboardSettingsMenu.SCANCODE_UP);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_LEFT, Apple2KeyboardSettingsMenu.SCANCODE_LEFT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_RIGHT, Apple2KeyboardSettingsMenu.SCANCODE_RIGHT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_DOWN, Apple2KeyboardSettingsMenu.SCANCODE_DOWN);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveRosettes(chars, scans);

                saveTouchDownKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
                saveSwipeSouthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
            }
        },
        AZ_LEFT_RIGHT_SPACE {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_az_left_right_space);
            }

            @Override
            public void apply(Apple2Activity activity) {
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'A', Apple2KeyboardSettingsMenu.SCANCODE_A);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_LEFT, Apple2KeyboardSettingsMenu.SCANCODE_LEFT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_RIGHT, Apple2KeyboardSettingsMenu.SCANCODE_RIGHT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'Z', Apple2KeyboardSettingsMenu.SCANCODE_Z);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveRosettes(chars, scans);

                saveTouchDownKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
                saveSwipeSouthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
            }
        },
        LEFT_RIGHT_SPACE {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_left_right_space);
            }

            @Override
            public void apply(Apple2Activity activity) {
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_LEFT, Apple2KeyboardSettingsMenu.SCANCODE_LEFT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.MOUSETEXT_RIGHT, Apple2KeyboardSettingsMenu.SCANCODE_RIGHT);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveRosettes(chars, scans);

                saveTouchDownKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
                saveSwipeSouthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
            }
        },
        IJKM_SPACE {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_ijkm_space);
            }

            @Override
            public void apply(Apple2Activity activity) {
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'I', Apple2KeyboardSettingsMenu.SCANCODE_I);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'J', Apple2KeyboardSettingsMenu.SCANCODE_J);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'K', Apple2KeyboardSettingsMenu.SCANCODE_K);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'M', Apple2KeyboardSettingsMenu.SCANCODE_M);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveRosettes(chars, scans);

                saveTouchDownKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
                saveSwipeSouthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
            }
        },
        WADX_SPACE {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_wadx_space);
            }

            @Override
            public void apply(Apple2Activity activity) {
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'W', Apple2KeyboardSettingsMenu.SCANCODE_W);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'A', Apple2KeyboardSettingsMenu.SCANCODE_A);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'D', Apple2KeyboardSettingsMenu.SCANCODE_D);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                addRosetteKey(chars, scans, 'X', Apple2KeyboardSettingsMenu.SCANCODE_X);
                addRosetteKey(chars, scans, Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveRosettes(chars, scans);

                saveTouchDownKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
                saveSwipeSouthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_NONACTION, -1);
            }
        },
        CRAZY_SEAFOX_KEYS {
            @Override
            public String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_preset_crazy_seafox);
            }

            @Override
            public void apply(Apple2Activity activity) {
                // Heh, the entire purpose of the keypad-variant touch joystick is to make this possible ;-)
                ArrayList<String> chars = new ArrayList<String>();
                ArrayList<String> scans = new ArrayList<String>();
                addRosetteKey(chars, scans, 'Y', Apple2KeyboardSettingsMenu.SCANCODE_Y);
                addRosetteKey(chars, scans, 'U', Apple2KeyboardSettingsMenu.SCANCODE_U);
                addRosetteKey(chars, scans, 'I', Apple2KeyboardSettingsMenu.SCANCODE_I);
                addRosetteKey(chars, scans, 'H', Apple2KeyboardSettingsMenu.SCANCODE_H);
                addRosetteKey(chars, scans, 'J', Apple2KeyboardSettingsMenu.SCANCODE_J);
                addRosetteKey(chars, scans, 'K', Apple2KeyboardSettingsMenu.SCANCODE_K);
                addRosetteKey(chars, scans, 'N', Apple2KeyboardSettingsMenu.SCANCODE_N);
                addRosetteKey(chars, scans, 'M', Apple2KeyboardSettingsMenu.SCANCODE_M);
                addRosetteKey(chars, scans, ',', Apple2KeyboardSettingsMenu.SCANCODE_COMMA);
                saveRosettes(chars, scans);

                saveTouchDownKey('D', Apple2KeyboardSettingsMenu.SCANCODE_D);
                saveSwipeSouthKey('F', Apple2KeyboardSettingsMenu.SCANCODE_F);
                saveSwipeNorthKey(Apple2KeyboardSettingsMenu.ICONTEXT_VISUAL_SPACE, Apple2KeyboardSettingsMenu.SCANCODE_SPACE);
            }
        };

        public static void addRosetteKey(ArrayList<String> chars, ArrayList<String> scans, int aChar, int aScan) {
            chars.add("" + aChar);
            scans.add("" + aScan);
        }

        public static void saveRosettes(ArrayList<String> chars, ArrayList<String> scans) {
            if (chars.size() != 9) {
                throw new RuntimeException("rosette chars is not correct size");
            }
            if (scans.size() != 9) {
                throw new RuntimeException("rosette scans is not correct size");
            }
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_ROSETTE_CHAR_ARRAY, new JSONArray(chars));
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_ROSETTE_SCAN_ARRAY, new JSONArray(scans));
        }

        public static void saveTouchDownKey(int aChar, int aScan) {
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_TOUCHDOWN_CHAR, aChar);
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_TOUCHDOWN_SCAN, aScan);
        }

        public static void saveSwipeSouthKey(int aChar, int aScan) {
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_SWIPE_SOUTH_CHAR, aChar);
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_SWIPE_SOUTH_SCAN, aScan);
        }

        public static void saveSwipeNorthKey(int aChar, int aScan) {
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_SWIPE_NORTH_CHAR, aChar);
            Apple2Preferences.setJSONPref(Apple2Preferences.PREF_DOMAIN_JOYSTICK, PREF_KPAD_SWIPE_NORTH_SCAN, aScan);
        }

        public abstract String getTitle(Apple2Activity activity);

        public abstract void apply(Apple2Activity activity);

        public static final int size = KeypadPreset.values().length;

        public static String[] titles(Apple2Activity activity) {
            String[] titles = new String[size];
            int i = 0;
            for (KeypadPreset preset : values()) {
                titles[i++] = preset.getTitle(activity);
            }
            return titles;
        }
    }

    enum SETTINGS implements Apple2AbstractMenu.IMenuEnum {
        KEYPAD_CHOOSE_KEYS {
            @Override
            public final String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_choose);
            }

            @Override
            public final String getSummary(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_choose_summary);
            }

            @Override
            public String getPrefKey() {
                return "kpPresetChoice";
            }

            @Override
            public Object getPrefDefault() {
                return KeypadPreset.IJKM_SPACE.ordinal() + 1;
            }

            @Override
            public final View getView(final Apple2Activity activity, View convertView) {
                convertView = _basicView(activity, this, convertView);
                _addPopupIcon(activity, this, convertView);
                return convertView;
            }

            @Override
            public void handleSelection(final Apple2Activity activity, final Apple2AbstractMenu settingsMenu, boolean isChecked) {
                final IMenuEnum self = this;
                String[] titles = new String[KeypadPreset.size + 1];
                titles[0] = activity.getResources().getString(R.string.keypad_preset_custom);
                System.arraycopy(KeypadPreset.titles(activity), 0, titles, 1, KeypadPreset.size);

                _alertDialogHandleSelection(activity, R.string.keypad_choose_title, titles, new IPreferenceLoadSave() {
                    @Override
                    public int intValue() {
                        return (int) Apple2Preferences.getJSONPref(self);
                    }

                    @Override
                    public void saveInt(int value) {
                        Apple2Preferences.setJSONPref(self, value);
                        if (value == 0) {
                            Apple2KeypadSettingsMenu keypadSettingsMenu = (Apple2KeypadSettingsMenu) settingsMenu;
                            keypadSettingsMenu.chooseKeys(activity);
                        } else {
                            KeypadPreset.values()[value - 1].apply(activity);
                        }
                    }
                });
            }
        },
        KEYPAD_CALIBRATE {
            @Override
            public final String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_calibrate);
            }

            @Override
            public final String getSummary(Apple2Activity activity) {
                return activity.getResources().getString(R.string.keypad_calibrate_summary);
            }

            @Override
            public void handleSelection(Apple2Activity activity, Apple2AbstractMenu settingsMenu, boolean isChecked) {
                ArrayList<Apple2MenuView> viewStack = new ArrayList<Apple2MenuView>();
                {
                    int idx = 0;
                    while (true) {
                        Apple2MenuView apple2MenuView = activity.peekApple2View(idx);
                        if (apple2MenuView == null) {
                            break;
                        }
                        viewStack.add(apple2MenuView);
                        ++idx;
                    }
                }

                Apple2JoystickCalibration calibration = new Apple2JoystickCalibration(activity, viewStack, Apple2SettingsMenu.TouchDeviceVariant.JOYSTICK_KEYPAD);

                // show this new view...
                calibration.show();

                // ...with nothing else underneath 'cept the emulator OpenGL layer
                for (Apple2MenuView apple2MenuView : viewStack) {
                    activity.popApple2View(apple2MenuView);
                }
            }
        },
        KEYPAD_ADVANCED {
            @Override
            public final String getTitle(Apple2Activity activity) {
                return activity.getResources().getString(R.string.settings_advanced);
            }

            @Override
            public final String getSummary(Apple2Activity activity) {
                return activity.getResources().getString(R.string.settings_advanced_joystick_summary);
            }

            @Override
            public void handleSelection(Apple2Activity activity, Apple2AbstractMenu settingsMenu, boolean isChecked) {
                new Apple2KeypadSettingsMenu.KeypadAdvanced(activity).show();
            }
        };

        public static final int size = SETTINGS.values().length;

        @Override
        public String getPrefDomain() {
            return Apple2Preferences.PREF_DOMAIN_JOYSTICK;
        }

        @Override
        public String getPrefKey() {
            return null;
        }

        @Override
        public Object getPrefDefault() {
            return null;
        }

        @Override
        public void handleSelection(Apple2Activity activity, Apple2AbstractMenu settingsMenu, boolean isChecked) {
        }

        @Override
        public View getView(Apple2Activity activity, View convertView) {
            return _basicView(activity, this, convertView);
        }

        public static String[] titles(Apple2Activity activity) {
            String[] titles = new String[size];
            int i = 0;
            for (SETTINGS setting : values()) {
                titles[i++] = setting.getTitle(activity);
            }
            return titles;
        }
    }

    // ------------------------------------------------------------------------
    // internals

    private void chooseKeys(Apple2Activity activity) {
        ArrayList<Apple2MenuView> viewStack = new ArrayList<Apple2MenuView>();
        {
            int idx = 0;
            while (true) {
                Apple2MenuView apple2MenuView = activity.peekApple2View(idx);
                if (apple2MenuView == null) {
                    break;
                }
                viewStack.add(apple2MenuView);
                ++idx;
            }
        }

        Apple2KeypadChooser chooser = new Apple2KeypadChooser(activity, viewStack);

        // show this new view...
        chooser.show();

        // ...with nothing else underneath 'cept the emulator OpenGL layer
        for (Apple2MenuView apple2MenuView : viewStack) {
            activity.popApple2View(apple2MenuView);
        }
    }

    public static class KeypadAdvanced extends Apple2AbstractMenu {

        private final static String TAG = "KeypadAdvanced";

        public KeypadAdvanced(Apple2Activity activity) {
            super(activity);
        }

        @Override
        public final String[] allTitles() {
            return SETTINGS.titles(mActivity);
        }

        @Override
        public final IMenuEnum[] allValues() {
            return SETTINGS.values();
        }

        @Override
        public final boolean areAllItemsEnabled() {
            return false;
        }

        @Override
        public final boolean isEnabled(int position) {
            if (position < 0 || position >= SETTINGS.size) {
                throw new ArrayIndexOutOfBoundsException();
            }
            return position == SETTINGS.JOYSTICK_ADVANCED.ordinal();
        }

        protected enum SETTINGS implements Apple2AbstractMenu.IMenuEnum {
            KEYREPEAT_THRESHOLD {
                @Override
                public final String getTitle(Apple2Activity activity) {
                    return "";
                }

                @Override
                public final String getSummary(Apple2Activity activity) {
                    return activity.getResources().getString(R.string.keypad_repeat_summary);
                }

                @Override
                public String getPrefKey() {
                    return "keyRepeatThresholdSecs";
                }

                @Override
                public Object getPrefDefault() {
                    return (float) 4 / KEYREPEAT_NUM_CHOICES;
                }

                @Override
                public View getView(final Apple2Activity activity, View convertView) {
                    final IMenuEnum self = this;
                    return _sliderView(activity, this, KEYREPEAT_NUM_CHOICES, new IPreferenceSlider() {
                        @Override
                        public void saveInt(int progress) {
                            Apple2Preferences.setJSONPref(self, (float) progress / KEYREPEAT_NUM_CHOICES);
                        }

                        @Override
                        public int intValue() {
                            return (int) (Apple2Preferences.getFloatJSONPref(self) * KEYREPEAT_NUM_CHOICES);
                        }

                        @Override
                        public void showValue(int progress, final TextView seekBarValue) {
                            seekBarValue.setText("" + ((float) progress / KEYREPEAT_NUM_CHOICES));
                        }
                    });
                }
            },
            JOYSTICK_ADVANCED {
                @Override
                public final String getTitle(Apple2Activity activity) {
                    return activity.getResources().getString(R.string.settings_advanced_joystick);
                }

                @Override
                public final String getSummary(Apple2Activity activity) {
                    return activity.getResources().getString(R.string.settings_advanced_joystick_summary);
                }

                @Override
                public void handleSelection(Apple2Activity activity, Apple2AbstractMenu settingsMenu, boolean isChecked) {
                    new Apple2JoystickSettingsMenu.JoystickAdvanced(activity).show();
                }
            };

            public static final int size = SETTINGS.values().length;

            @Override
            public String getPrefDomain() {
                return Apple2Preferences.PREF_DOMAIN_JOYSTICK;
            }

            @Override
            public String getPrefKey() {
                return null;
            }

            @Override
            public Object getPrefDefault() {
                return null;
            }

            @Override
            public void handleSelection(Apple2Activity activity, Apple2AbstractMenu settingsMenu, boolean isChecked) {
            }

            @Override
            public View getView(Apple2Activity activity, View convertView) {
                return _basicView(activity, this, convertView);
            }

            public static String[] titles(Apple2Activity activity) {
                String[] titles = new String[size];
                int i = 0;
                for (SETTINGS setting : values()) {
                    titles[i++] = setting.getTitle(activity);
                }
                return titles;
            }
        }
    }
}


/*
 * Apple // emulator for *nix
 *
 * This software package is subject to the GNU General Public License
 * version 3 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * Copyright 2017 Aaron Culliney
 *
 */

package org.deadc0de.apple2ix;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.support.annotation.Nullable;
import android.util.Log;

import java.util.concurrent.atomic.AtomicBoolean;

public class Apple2DiskChooserActivity extends Activity {

    public static final AtomicBoolean sDiskChooserIsChoosing = new AtomicBoolean(false);
    public static Callback sDisksCallback;

    public interface Callback {
        void onDisksChosen(DiskArgs args);
    }

    @Nullable
    public static ParcelFileDescriptor openFileDescriptorFromUri(Context ctx, Uri uri) {

        ParcelFileDescriptor pfd = null;

        try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
                throw new RuntimeException("SDK Version not allowed access");
            }

            if (!DocumentsContract.isDocumentUri(ctx, uri)) {
                throw new RuntimeException("Not a Document URI for " + uri);
            }

            ContentResolver resolver = ctx.getContentResolver();
            resolver.takePersistableUriPermission(uri, (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION));
            pfd = resolver.openFileDescriptor(uri, "rw");
        } catch (Throwable t) {
            Log.e(TAG, "OOPS, could not get appropriate access to URI ( " + uri + " ) : " + t);
        }

        return pfd;
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putBoolean("ran", true);
    }

    @Override
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);

        Bundle b;
        {
            Intent intent = getIntent();
            Bundle extras = null;
            if (intent != null) {
                extras = intent.getExtras();
            }

            if (savedState != null) {
                b = savedState;
            } else if (extras != null) {
                b = extras;
            } else {
                b = new Bundle();
            }
        }

        boolean ran = b.getBoolean("ran");
        if (ran) {
            Log.e(TAG, "OOPS, already ran...");
            finish();
            return;
        }

        ////Intent pickIntent = new Intent(Intent.ACTION_GET_CONTENT);
        Intent pickIntent = new Intent(Intent.ACTION_OPEN_DOCUMENT);

        try {
            pickIntent.setType("*/*");
            pickIntent.addCategory(Intent.CATEGORY_OPENABLE);

            /* FIXME TODO : currently we don't have decent UI/UX for multi-select ...
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                pickIntent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
            }*/

            if (!ran) {
                startActivityForResult(pickIntent, EDIT_REQUEST_CODE);
            }
        } catch (Throwable t) {
            Log.e(TAG, "OOPS : " + t);
            setResult(RESULT_CANCELED);
            finish();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode != RESULT_CANCELED) {

            /* FIXME TODO : currently we don't have decent UI/UX for multi-select ...
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {

                ClipData clipData = null;
                if (data != null) {
                    clipData = data.getClipData();
                }

                if (clipData != null && clipData.getItemCount() > 1) {
                    uris = new Uri[2];
                    uris[0] = clipData.getItemAt(0).getUri();
                    uris[1] = clipData.getItemAt(1).getUri();
                }
            }*/

            if (chosenUri == null) {
                if (data != null) {
                    chosenUri = data.getData();
                }
            }

            if (chosenUri != null) {
                chosenPfd = openFileDescriptorFromUri(this, chosenUri);
            }
        }

        setResult(RESULT_OK);
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void finish() {
        sDiskChooserIsChoosing.set(false);
        String name = chosenUri == null ? "" : chosenUri.toString();
        if (sDisksCallback != null) {
            sDisksCallback.onDisksChosen(new DiskArgs(name, chosenUri, chosenPfd));
        }
        super.finish();
    }

    private Uri chosenUri;

    private ParcelFileDescriptor chosenPfd;

    private static final String TAG = "A2DiskChooserActivity";

    private static final int EDIT_REQUEST_CODE = 44;
}

class DiskArgs {
    public String name;
    public String path;
    public Uri uri;
    public ParcelFileDescriptor pfd;

    public DiskArgs(String name, Uri uri, ParcelFileDescriptor pfd) {
        this.name = name;
        this.uri = uri;
        this.pfd = pfd;
    }

    public DiskArgs(String path) {
        this.path = path;
    }
}

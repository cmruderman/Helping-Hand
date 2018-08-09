//
//  ViewController.swift
//  DemoApp
//
//  Created by Stefano Vettor on 28/03/16.
//  Copyright © 2016 Stefano Vettor. All rights reserved.
//
//  Modified by Joshua Girard for UMass ECE SDP '18
//

import UIKit
import MjpegStreamingKit

class ViewController: UIViewController {
    
    @IBOutlet weak var loadingIndicator: UIActivityIndicatorView!
    
    //@IBOutlet weak var playButton: UIButton!
    
    
    @IBOutlet weak var leftImageView: UIImageView!
    @IBOutlet weak var rightImageView: UIImageView!
    
    var leftUrl: URL?
    var rightUrl: URL?
    
    //var playing = false
    
    var leftStreamingController: MjpegStreamingController!
    var rightStreamingController: MjpegStreamingController!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        leftStreamingController = MjpegStreamingController(imageView: leftImageView)
        leftStreamingController.didStartLoading = { [unowned self] in
            self.loadingIndicator.startAnimating()
        }
        leftStreamingController.didFinishLoading = { [unowned self] in
            self.loadingIndicator.stopAnimating()
        }
        
        leftUrl = URL(string: "http://192.168.1.6:8081/?action=stream")
        leftStreamingController.contentURL = leftUrl
        
        rightStreamingController = MjpegStreamingController(imageView: rightImageView)
        rightStreamingController.didStartLoading = { [unowned self] in
            self.loadingIndicator.startAnimating()
        }
        rightStreamingController.didFinishLoading = { [unowned self] in
            self.loadingIndicator.stopAnimating()
        }
        
        rightUrl = URL(string: "http://192.168.1.6:8082/?action=stream")
        rightStreamingController.contentURL = rightUrl
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
    
//    @IBAction func playAndStop(sender: AnyObject) {
//        if playing {
//            playButton.setTitle("Play", for: .normal)
//            leftStreamingController.stop()
//            rightStreamingController.stop()
//            playing = false
//        } else {
//            playButton.removeFromSuperview()
//            leftStreamingController.play()
//            rightStreamingController.play()
//            playing = true
//            //playButton.setTitle("Stop", for: .normal)
//        }
//    }
    
    
}


